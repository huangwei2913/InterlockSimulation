#include "InterlockControl.h"
#include <QDebug>
#include "DataAccessLayer.h"
#include "BlinkingSquare.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "Utilty.h"
#include "SwitchDirectOperationEvent.h"
#include "SwitchReverseOperationEvent.h"
#include "BlinkingSquare.h"
#include <QQueue>
#include <QDebug>
#include "DataAccessLayer.h"
#include "Phy_turnout.h"
#include "TrackView.h"
#include <QStateMachine>
#include <QState>
#include "CommandStatusChecker.h"
#include "PhyDeviceManager.h"
InterlockControl::~InterlockControl()
{
}
InterlockControl::InterlockControl(QObject* parent, TrackView* trackView) : QObject(parent), m_trackView(trackView)
{
    consolidateSwitchMap.insert("1", "3");
    consolidateSwitchMap.insert("3", "1");
    consolidateSwitchMap.insert("5", "7");
    consolidateSwitchMap.insert("7", "5");
    m_counter = 0;
    m_statusChecker = new CommandStatusChecker(this);
    connect(&da, &DataAccessLayer::comandInserted, m_statusChecker, &CommandStatusChecker::startCheckingRecords); //先插入命令
    connect(m_statusChecker, &CommandStatusChecker::checkRecordStatus, &da, &DataAccessLayer::checkComandStatusById); //再检查命令表状态
    connect(&da, &DataAccessLayer::commandStatusChecked, this, &InterlockControl::handleRecordStatusChecked);
    //这个状态机实现对道岔转辙机的操作
    m_stateMachine_switchDC = new QStateMachine(this);
    m_requestingState_DCSwitch = new QState(m_stateMachine_switchDC);
    m_checkingState_DCSwitch = new QState(m_stateMachine_switchDC);
    m_successState_DCSwitch = new QState(m_stateMachine_switchDC);      //我们也许在这里可以记录和控制什么
    // 设置状态转换
    m_requestingState_DCSwitch->addTransition(this, &InterlockControl::switchDirectOperationRequested, m_checkingState_DCSwitch);
    m_checkingState_DCSwitch->addTransition(this, &InterlockControl::switchDirectOperationSucceeded, m_successState_DCSwitch);
    m_checkingState_DCSwitch->addTransition(this, &InterlockControl::switchDirectOperationFailed, m_checkingState_DCSwitch);
    m_stateMachine_switchDC->setInitialState(m_requestingState_DCSwitch);
    //这个状态机实现对转辙机的的反操操作
    m_stateMachine_switchFC = new QStateMachine(this);
    m_requestingState_FCSwitch = new QState(m_stateMachine_switchFC);
    m_checkingState_FCSwitch = new QState(m_stateMachine_switchFC);
    m_successState_FCSwitch = new QState(m_stateMachine_switchFC);      //我们也许在这里可以记录和控制什么
    // 设置状态转换
    m_requestingState_FCSwitch->addTransition(this, &InterlockControl::switchReverseOperationRequested, m_checkingState_FCSwitch);
    m_checkingState_FCSwitch->addTransition(this, &InterlockControl::switchReverseOperationSucceeded, m_successState_FCSwitch);
    m_checkingState_FCSwitch->addTransition(this, &InterlockControl::switchReverseOperationFailed, m_checkingState_FCSwitch);
    m_stateMachine_switchFC->setInitialState(m_requestingState_FCSwitch);
}
void InterlockControl::handleRecordStatusChecked(QString routepath, int cmdID, bool isValid) {
    if (isValid) {
        // 记录状态变更为有效，这里还有一个隐藏的bug就是只能针对某个转辙机进行检查
        //将进路信息填充到，在这里还是预建立的进路信息
        da.insertInterlockingRecord(routepath);
        emit turnoutPassChecked(routepath);
        //这里我们根据命令ID去查询，来看到底是哪个方面的指令，例如，有可能是驱动道岔之后，等待道岔反馈的，
       //有的是驱动轨道区段方向继电器变化的
        //首先，我们会读取该命令的内容，显然这个是不大的
        auto commandcontent = da.getCommandContentById(cmdID);
        //然后判断是不是特定类型的命令
        QByteArray ba;
        ba.resize(3);
        ba[0] = 0xc8;
        ba[1] = 0x80;
        ba[2] = 0x8e;
        QString t_command = QString::fromStdString(ba.toStdString());
        if (t_command == commandcontent) {
            emit sectionDirectionPassChecked(routepath);
        }
    }
    else {
        // 记录状态未变更为有效
    }
}
void InterlockControl::buttonClicked(QString buttonName)
{
    ++m_counter;
    qDebug() << "InterlockControl::buttonClicked called for button: " << buttonName << " mcount=" << m_counter;
    m_queue.enqueue(buttonName);
    if (m_counter >= 2) {
        QString firstButton = m_queue.dequeue();
        QString secondButton = m_queue.dequeue();
        if (firstButton != secondButton) {
            //这里先插入到数据库中，然后再发出消息
            auto startsignal = firstButton.split('-').first();
            auto endsignal = secondButton.split('-').first();
            da.recordRoute(startsignal, endsignal);
            emit differentButtonClicked();
        }
        else {
            emit sameButtonClicked();
        }
        m_counter = 0;  // 重置计数器
    }
}
//针对所要办理的进路，我们需要操作指定的道岔
void InterlockControl::operateTurnouts(QString routepath)
{
    qDebug() << "开始操作道岔，但界面中道岔的状态先不修改..." << routepath;
    //首先是通知视图控制层不允许操作有关的道岔
    QString startsignalname;
    QString endsignalname;
    //找到需要停止的按钮
    auto switchpostionmap = da.getAllTurnoutPositions();
    auto signalpotionmap = da.getAllSignalPositions();
    QStringList parts = routepath.split("->", Qt::SkipEmptyParts);
    if (!parts.isEmpty()) {
        // 获取第一个字符
        QString firstChar = parts.first().trimmed();
        // 获取最后一个字符
        QString lastPart = parts.last().trimmed();
        QString lastChar = lastPart.right(2); // 获取最后两个字符
        qDebug() << "第一个字符: " << firstChar;
        qDebug() << "最后一个字符: " << lastChar;
        startsignalname = firstChar;
        endsignalname = lastChar;
        QString buttonName = firstChar + "-A1";
        auto the_button = this->m_trackView->m_blinksquareMap.value(buttonName);
        the_button->stopBliking();
        QString buttonName1 = lastChar + "-A1";
        auto the_button1 = this->m_trackView->m_blinksquareMap.value(buttonName1);
        qDebug() << "MD...." << the_button1->getbuttonName();
        the_button1->stopBliking();
    }
    else {
        qDebug() << "无法找到分割的字符";
    }
    QStringList routeSegments = routepath.split("->", Qt::SkipEmptyParts); // 以 "->" 分割字符串并存储在 QStringList 中
    QStringList numbers; // 存储纯数字部分
    for (const QString& segment : routeSegments) {
        QString trimmedSegment = segment.trimmed(); // 去除前后空格
        bool isAllDigits = !trimmedSegment.isEmpty() && trimmedSegment.toInt(); // 检查是否为纯数字
        if (isAllDigits) {
            qDebug() << "添加数字...:" << trimmedSegment; // 打印当前要添加的数字
            numbers.append(trimmedSegment);
        }
    }
    emit notifyViewControllerLockSwitch(numbers); //通知视图中的对象不要操作这几个道岔
    QMap<QString, QString> turnoutDemandStatus;  //'Lost', 'Normal', 'Reverse'
    for (QString tmpnumber : numbers) {
        QString prev, next;
        findAdjacentStrings(routepath, tmpnumber, prev, next);
        qDebug() << "前面和后面一个...:" << prev << next << "当前" << tmpnumber; // 打印当前要添加的
        auto p = switchpostionmap.value(tmpnumber);
        QPointF p1;
        QRegExp rx("^\\d+$");
        if (rx.exactMatch(prev)) {
            p1 = switchpostionmap.value(prev);
        }
        else {
            p1 = signalpotionmap.value(prev);
        }
        QPointF p2;
        if (rx.exactMatch(next)) {
            p2 = switchpostionmap.value(next);
        }
        else {
            p2 = signalpotionmap.value(next);
        }
        auto t_angle = calculateAngleBetweenLineSegments(p1, p, p, p2);
        qDebug() << "the angle is ........." << t_angle;
        if (t_angle < 20) {
            turnoutDemandStatus.insert(tmpnumber, "Normal");
        }
        else {
            turnoutDemandStatus.insert(tmpnumber, "Reverse");
        }
    }
    QMap<QString, QString> tempMap;
    qDebug() << "我们希望多少个转辙机到达什么状态...." << turnoutDemandStatus.size();
    // 遍历 turnoutDemandStatus 的每个键
    for (auto it = turnoutDemandStatus.begin(); it != turnoutDemandStatus.end(); ++it) {
        const QString& key = it.key();
        const QString& value = it.value();
        // 在 consolidateSwitchMap 中查找相应键对应的值
        if (consolidateSwitchMap.contains(key)) {
            QString linkedValue = consolidateSwitchMap.value(key);
            // 将找到的值插入到临时 map 中
            tempMap.insert(linkedValue, value);
            qDebug() << "联动信号机." << linkedValue << "要达到的状态" << value;
        }
    }
    turnoutDemandStatus.unite(tempMap);
    //
    QJsonObject jsonObject;
    for (auto it = turnoutDemandStatus.begin(); it != turnoutDemandStatus.end(); ++it) {
        jsonObject.insert(it.key(), it.value());
    }
    QJsonDocument jsonDocument(jsonObject);
    QString jsonString = jsonDocument.toJson(QJsonDocument::Compact);
    da.updateRouteDescription(startsignalname, endsignalname, jsonString);
    qDebug() << "要对什么进路进行操作，并让其对应的转辙机处于什么状态" << routepath << jsonString;
    emit notifyTrackCommanderSwitchOperation(routepath, jsonString);
    return;
}
//在这个里面应该要检测所发送命令的状态
void InterlockControl::checkSwitchLocation(QString route_path, QString switchmap) {
    QJsonDocument jsonDocument = QJsonDocument::fromJson(switchmap.toUtf8());
    QJsonObject jsonObject = jsonDocument.object();
    // 将 QJsonObject 转换为 QMap
    QMap<QString, QString> tempMap;
    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
        tempMap.insert(it.key(), it.value().toString());
    }
    QVector<int> commandIDlsit; //到后期我们来检查这些数据库的字段是否满足要求
    //这里就用这样的逻辑来搞算了
    for (auto it = tempMap.begin(); it != tempMap.end(); ++it) {
        const QString& key = it.key();
        const QString& value = it.value();
        if (it.key() == '4') {
            QByteArray temp_data;
            temp_data.resize(3);
            temp_data[0] = 0xc0;
            temp_data[1] = 0x80;
            temp_data[2] = 0x8e;
            auto zz = temp_data.toStdString();
            da.insertCommandAndGetId(route_path, zz);
            // auto commandId = da.insertCommandAndGetId(zz);
            // commandIDlsit.append(commandId);
        }
        else {
            QByteArray temp_data;
            temp_data.resize(3);
            temp_data[0] = 0xc2;
            temp_data[1] = 0x80;
            temp_data[2] = 0x8e;
            auto zz = temp_data.toStdString();
            // auto commandId = da.insertCommandAndGetId(zz);
            //commandIDlsit.append(commandId);
            da.insertCommandAndGetId(route_path, zz);
        }
    }
    //然后启动计时器
}
//基于所订阅的方向来检查进路所涉及到的区段是否需要改变方向,如果需要则发送站内改方指令
void InterlockControl::checkTrackSectionDirection(QString route_path) {
    //现在我们必须用算法来解决判断条件的问题，一般来说，如果进路只是涉及下行线路正向运行的话，进路所涉及到的节点的(x,y)坐标值的变化规律是x保持变大，y保持不变或者变小
    // 反之，如果是下行反向运行的话，进路中所涉及到的所有节点都满足，运行方向前一个节点的x值保持变小，而y保持不变或者变小
    //如果是在上行线路正向形式的话，进路所涉及的节点的(x,y)坐标值的变化规律是x保持变小，而y不变或者y的坐标值变小。反之，如果在上行线路上反向形式的话
    //x坐标值持续变大，y坐标值保持不变或者变大。
    //但有一种非常特殊的情况是，如果所经过的路线中包含互相联动的转辙机的时候，这要对除了这对联动转辙机包含的路径之外的，进路中的所有子路径（这个要仔细理解，也就是排除类似于5,7 7,5 ,3,1 ,1,3路径之外的），也就是3-11DG和1-9DG都要进行改方判断。到底需要改变站内哪些区段的FQJ，主要是看进路方向中的x和y的额变化规律，例如，在下行线路反向运行时候，例如从SI->11->3->5->X运行的时候，我们会发现在这些子路径Si->11 11->3这个过程中， X的值不断减小，Y值不变后者变大，那么就说明我们需要改变所经过区段的FQJ了。其实就是这么回事，对下行或者上行轨道方向进行变化的按钮是一组显示框中的改方按钮，另外，对下行线路改方是需要通过0x91命令，将XFJ变成0，对上行线路改方这是需要通过0x90命令，将S-FJ变成0
    QVector<QString> trackneedchangesdirection; //就是需要改变的轨道区段
    //第一步：知道了进路前进方向
    auto switchpistionmap = da.getAllTurnoutPositions();
    auto signalPositionmap = da.getAllSignalPositions();
    auto direction = getRouteDirection(route_path, switchpistionmap, signalPositionmap);
    //首先对不是交叉运行的线路进行分析
    if (direction == RouteDirection::DownwardForward
        || direction == RouteDirection::DownwardReverse) {
        //从数据库中获取当前下行线到底是什么方向
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Down");
        if ((t_trackstatus == "Forward" && direction == RouteDirection::DownwardReverse) ||
            (t_trackstatus == "Reverse" && direction == RouteDirection::DownwardForward)) {
            auto wesections = getTrackSectionTakenByRoutepath(route_path); //进路经过的区段
            QByteArray tempba;
            tempba.resize(3);
            tempba[0] = 0x98;
            QString data1 = "11111111";
            QString data2 = "1111";
            QStringList tmpsectionss = { "3G", "4DG", "IG", "3-11DG" };
            for (const QString& section : tmpsectionss)
            {
                if (wesections.contains(section)) {
                    trackneedchangesdirection.append(section);
                }
                data2 += wesections.contains(section) ? "0" : "1";
            }
            tempba[1] = Utilty::convertStrToByte(data1.trimmed());;
            tempba[2] = Utilty::convertStrToByte(data2.trimmed());;
            auto t_command = Utilty::appendCRC(tempba);
            da.insertCommand(t_command);
        }
    }
    if (direction == RouteDirection::UpwardForward || direction == RouteDirection::UpwardReverse) {
        //从数据库中获取当前下行线到底是什么方向
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Up");
        if ((t_trackstatus == "Forward" && direction == RouteDirection::UpwardReverse) || (t_trackstatus == "Reverse" && direction == RouteDirection::UpwardForward)) {
            auto wesections = getTrackSectionTakenByRoutepath(route_path);
            QByteArray tempba;
            tempba.resize(3);
            tempba[0] = 0x9a;
            QString data1 = "11111111";
            QString data2 = "1111";
            QStringList tmpsectionss = { "4G", "2DG", "IIG", "1-9DG" };
            for (const QString& section : tmpsectionss)
            {
                if (wesections.contains(section)) {
                    trackneedchangesdirection.append(section);
                }
                data2 += wesections.contains(section) ? "0" : "1";
            }
            tempba[1] = Utilty::convertStrToByte(data1.trimmed());;
            tempba[2] = Utilty::convertStrToByte(data2.trimmed());;
            auto t_command = Utilty::appendCRC(tempba);
            da.insertCommand(t_command);
        }
    }
    if (direction == RouteDirection::CrossLine) {
        //对于跨行的进路，分开进行讨论
        auto subpathss = findSubPathsForCrossLineRoute(route_path);
        for (QString subpath : subpathss) {
            auto t_trackdirection = getLineDirectionFromPath(subpath);
            if (t_trackdirection == TrackDirection::Downward) { //下行线
                auto t_routedirdction = this->getRouteDirectionForDownlinePath(subpath, switchpistionmap, signalPositionmap);
                if (t_routedirdction == RouteDirection::DownwardForward) { //进路方向是下行正向行驶
                    //查看当前线路方向,如果与其方向相反，
                    auto t_trackstatus = da.getCurrentTrackDirectionStatus("Down");
                    if (t_trackstatus == "Reverse") { //则需要修改区段的改方
                        auto wesections = getTrackSectionTakenByRoutepath(route_path);
                        QByteArray tempba;
                        tempba.resize(3);
                        tempba[0] = 0x98;
                        QString data1 = "11111111";
                        QString data2 = "1111";
                        QStringList tmpsectionss = { "3G", "4DG", "IG", "3-11DG" };
                        for (const QString& section : tmpsectionss)
                        {
                            if (wesections.contains(section)) {
                                trackneedchangesdirection.append(section);
                            }
                            data2 += wesections.contains(section) ? "0" : "1";
                        }
                        tempba[1] = Utilty::convertStrToByte(data1.trimmed());;
                        tempba[2] = Utilty::convertStrToByte(data2.trimmed());;
                        auto t_command = Utilty::appendCRC(tempba);
                        da.insertCommand(t_command);
                    }
                }
                else { //如果进路方向是下行反向
                    auto t_trackstatus = da.getCurrentTrackDirectionStatus("Down");
                    if (t_trackstatus == "Forward") { //则需要修改区段的改方
                        auto wesections = getTrackSectionTakenByRoutepath(route_path);
                        QByteArray tempba;
                        tempba.resize(3);
                        tempba[0] = 0x98;
                        QString data1 = "11111111";
                        QString data2 = "1111";
                        QStringList tmpsectionss = { "3G", "4DG", "IG", "3-11DG" };
                        for (const QString& section : tmpsectionss)
                        {
                            if (wesections.contains(section)) {
                                trackneedchangesdirection.append(section);
                            }
                            data2 += wesections.contains(section) ? "0" : "1";
                        }
                        tempba[1] = Utilty::convertStrToByte(data1.trimmed());;
                        tempba[2] = Utilty::convertStrToByte(data2.trimmed());;
                        auto t_command = Utilty::appendCRC(tempba);
                        da.insertCommand(t_command);
                    }
                }
            }
            else {  //如果是上行线，
                auto t_routedirdction = this->getRouteDirectionForUplinePath(subpath, switchpistionmap, signalPositionmap);
                if (t_routedirdction == RouteDirection::UpwardForward) { //如果进路是上行正向
                    auto t_trackstatus = da.getCurrentTrackDirectionStatus("Up");
                    if (t_trackstatus == "Reverse") { //则需要修改区段的改方
                        auto wesections = getTrackSectionTakenByRoutepath(route_path);
                        QByteArray tempba;
                        tempba.resize(3);
                        tempba[0] = 0x9a;
                        QString data1 = "11111111";
                        QString data2 = "1111";
                        QStringList tmpsectionss = { "4G", "2DG", "IIG", "1-9DG" };
                        for (const QString& section : tmpsectionss)
                        {
                            if (wesections.contains(section)) {
                                trackneedchangesdirection.append(section);
                            }
                            data2 += wesections.contains(section) ? "0" : "1";
                        }
                        tempba[1] = Utilty::convertStrToByte(data1.trimmed());;
                        tempba[2] = Utilty::convertStrToByte(data2.trimmed());;
                        auto t_command = Utilty::appendCRC(tempba);
                        da.insertCommand(t_command);
                    }
                }
                else { //进路是上行反向
                    auto t_trackstatus = da.getCurrentTrackDirectionStatus("Up");
                    if (t_trackstatus == "Forward") { //则需要修改区段的改方
                        auto wesections = getTrackSectionTakenByRoutepath(route_path);
                        QByteArray tempba;
                        tempba.resize(3);
                        tempba[0] = 0x9a;
                        QString data1 = "11111111";
                        QString data2 = "1111";
                        QStringList tmpsectionss = { "4G", "2DG", "IIG", "1-9DG" };
                        for (const QString& section : tmpsectionss)
                        {
                            if (wesections.contains(section)) {
                                trackneedchangesdirection.append(section);
                            }
                            data2 += wesections.contains(section) ? "0" : "1";
                        }
                        tempba[1] = Utilty::convertStrToByte(data1.trimmed());;
                        tempba[2] = Utilty::convertStrToByte(data2.trimmed());;
                        auto t_command = Utilty::appendCRC(tempba);
                        da.insertCommand(t_command);
                    }
                }
            }
        }
    }
    //
    emit needValidateTrackSectionDirection(route_path, trackneedchangesdirection);
} // 
RouteDirection InterlockControl::getRouteDirection(const QString& routePath, const QMap<QString, QPointF>& switchPositionMap, const QMap<QString, QPointF>& signalPositionMap) {
    QStringList pathElements = routePath.split(" -> ");
    QList<QPointF> positions;
    for (const QString& element : pathElements)
    {
        auto t_element = element.trimmed();
        bool ok;
        t_element.toInt(&ok);
        if (ok)
        {
            // 转辙机
            if (switchPositionMap.contains(t_element))
            {
                positions.append(switchPositionMap[t_element]);
            }
        }
        else
        {
            // 信号机
            if (signalPositionMap.contains(t_element))
            {
                positions.append(signalPositionMap[t_element]);
            }
        }
    }
    bool isCrossLine = false;
    for (int i = 1; i < pathElements.size(); i++)
    {
        QString current = pathElements[i].trimmed();
        QString prev = pathElements[i - 1].trimmed();
        if (consolidateSwitchMap.contains(current) && consolidateSwitchMap[current] == prev)
        {
            // 跨线运行
            isCrossLine = true;
            break;
        }
    }
    if (isCrossLine)
    {
        return RouteDirection::CrossLine;
    }
    auto startsignal = pathElements.first().trimmed();
    auto endsignal = pathElements.last().trimmed();
    auto sections = da.findMatchingRoute(startsignal, endsignal).last();
    QStringList downlinksectionlist = { "3-11DG","IG","4DG","3G" };
    QStringList uplinksectionlist = { "1-9DG","IIG","2DG","4G" };
    bool isDownward = false;
    bool isUpward = false;
    QRegularExpression re("'(.*?)'");
    QRegularExpressionMatchIterator matchIterator = re.globalMatch(sections);
    QStringList sectionList;
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        QString trackSection = match.captured(1); // 获取匹配的轨道区段名称
        qDebug() << trackSection;
        sectionList.append(trackSection);
    }
    for (const QString& section : sectionList)
    {
        if (downlinksectionlist.contains(section))
        {
            isDownward = true;
        }
        else if (uplinksectionlist.contains(section))
        {
            isUpward = true;
        }
        else {
            continue;
        }
    }
    bool allMatchDirection = true;
    if (isDownward)
    {
        for (int i = 0; i < positions.size() - 1; i++)
        {
            const QPointF& curr = positions[i];
            const QPointF& next = positions[i + 1];
            if (next.x() > curr.x() && (next.y() <= curr.y() || qFuzzyCompare(next.y(), curr.y())))
            {
                // 下行正向
            }
            else if (next.x() < curr.x() && (next.y() <= curr.y() || qFuzzyCompare(next.y(), curr.y())))
            {
                // 上行反向
                allMatchDirection = false;
                break;
            }
            else
            {
                allMatchDirection = false;
                break;
            }
        }
        if (allMatchDirection)
        {
            return RouteDirection::DownwardForward;
        }
        else
        {
            return RouteDirection::DownwardReverse;
        }
    }
    allMatchDirection = true;
    if (isUpward)
    {
        for (int i = 0; i < positions.size() - 1; i++)
        {
            const QPointF& curr = positions[i];
            const QPointF& next = positions[i + 1];
            if (next.x() < curr.x() && (next.y() <= curr.y() || qFuzzyCompare(next.y(), curr.y())))
            {
                // 上行反向
            }
            else if (next.x() > curr.x() && (next.y() <= curr.y() || qFuzzyCompare(next.y(), curr.y())))
            {
                // 下行正向
                allMatchDirection = false;
                break;
            }
            else
            {
                allMatchDirection = false;
                break;
            }
        }
        if (allMatchDirection)
        {
            return RouteDirection::UpwardReverse;
        }
        else
        {
            return RouteDirection::UpwardForward;
        }
    }
    return RouteDirection::DownwardForward; //will not exectuted here
}
QStringList InterlockControl::getTrackSectionTakenByRoutepath(QString routePath) {
    QStringList pathElements = routePath.split(" -> ");
    auto startsignal = pathElements.first().trimmed();
    auto endsignal = pathElements.last().trimmed();
    auto sections = da.findMatchingRoute(startsignal, endsignal).last();
    QRegularExpression re("'(.*?)'");
    QRegularExpressionMatchIterator matchIterator = re.globalMatch(sections);
    QStringList sectionList;
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        QString trackSection = match.captured(1); // 获取匹配的轨道区段名称
        qDebug() << trackSection;
        sectionList.append(trackSection);
    }
    return sectionList;
}
QStringList InterlockControl::findSubPathsForCrossLineRoute(const QString& routePath)
{
    QStringList pathElements = routePath.split(" -> ");
    QStringList subPaths;
    QString lockSwitch = "";
    for (int i = 0; i < pathElements.size(); i++)
    {
        QString element = pathElements[i].trimmed();
        if (consolidateSwitchMap.contains(element))
        {
            // 找到第一个联动转辙机
            lockSwitch = element;
            break;
        }
    }
    if (lockSwitch.isEmpty())
    {
        // 没有找到联动转辙机,返回原始进路
        subPaths << routePath;
        return subPaths;
    }
    // 从开始到第一个联动转辙机的子路径
    QStringList subPath1;
    for (int i = 0; i <= pathElements.indexOf(lockSwitch); i++)
    {
        subPath1 << pathElements[i];
    }
    subPaths << subPath1.join(" -> ");
    // 从与联动转辙机对应的转辙机开始到进路结束的子路径
    QStringList subPath2;
    int lockSwitchIndex = pathElements.indexOf(lockSwitch);
    QString correspondingSwitch = consolidateSwitchMap[lockSwitch];
    for (int i = pathElements.indexOf(correspondingSwitch); i < pathElements.size(); i++)
    {
        subPath2 << pathElements[i];
    }
    subPaths << subPath2.join(" -> ");
    return subPaths;
}
void InterlockControl::checkSignalColorStatus(QString routepath, QStringList signallist, QStringList colorList, QMap<QString, int>whichIDs) {
    //首先，我们来查看数据库中的记录是否
    bool commanstatusok = true;
    QList<int> values = whichIDs.values();
    for (int i = 0; i < values.size(); ++i) {
        if (!da.iscomandRecordValid(values[i])) {
            commanstatusok = false;
            break;
        }
    }
    if (commanstatusok == true) {  //接着，我们需要在PhyDeviceManager类中查询所有信号机的颜色是否驱动到位了
        bool passedchecked = true;
        for (int i = 0; i < signallist.size(); i++) {
            auto physignalstatus = m_phydevicemanager->getLastedColorForSignal(signallist.at(i));
            auto thetargetcolor = colorList.at(i);
            auto comparedphysignalstatus = physignalstatus->getLight1() + physignalstatus->getLight2();
            if (comparedphysignalstatus.trimmed() != thetargetcolor) {
                passedchecked = false;
                break;
            }
        }
        if (passedchecked == true) {
            emit conductTrackFamaRequest(routepath);
        }
    }
} //我们需要首先查询数据库中的记录是否已经成功
QStringList InterlockControl::getForwardSignalsWithSpecificInRoute(QString routepath) {
    auto the_last_signal = routepath.split(" -> ").last().trimmed(); //找到最后一个信号机
    auto the_first_signal = routepath.split(" -> ").first().trimmed();
    auto the_signal_inwhichline = Configurations::determineSignalType(the_last_signal);
    QStringList avaibleSignals;  //所有可能遍历到的信号机序列
    auto signalPositionMap = da.getAllSignalPositions();
    if (the_signal_inwhichline == SignalType::DownlineSignal) {
        //这里需要判断起始信号机的x坐标值
        auto the_startsignalpos = signalPositionMap.value(the_first_signal);
        auto the_lastsignalpos = signalPositionMap.value(the_last_signal);
        if (the_startsignalpos.x() < the_lastsignalpos.x()) {
            avaibleSignals = Configurations::downlineTrackTraversing; //我们要怎么遍历
        }
        else {
            QStringList reversedList = Configurations::downlineTrackTraversing;
            std::reverse(reversedList.begin(), reversedList.end());
            avaibleSignals = reversedList;
        }
    }
    if (the_signal_inwhichline == SignalType::DownwardCXSignal) {
        auto the_startsignalpos = signalPositionMap.value(the_first_signal);
        auto the_lastsignalpos = signalPositionMap.value(the_last_signal);
        if (the_startsignalpos.x() < the_lastsignalpos.x()) {
            avaibleSignals = Configurations::downlineTrackTraversingCX; //我们要怎么遍历
        }
        else {
            QStringList reversedList = Configurations::downlineTrackTraversingCX;
            std::reverse(reversedList.begin(), reversedList.end());
            avaibleSignals = reversedList;
        }
    }
    if (the_signal_inwhichline == SignalType::UplineSignal) {
        //这里需要判断起始信号机的x坐标值
        auto the_startsignalpos = signalPositionMap.value(the_first_signal);
        auto the_lastsignalpos = signalPositionMap.value(the_last_signal);
        if (the_startsignalpos.x() > the_lastsignalpos.x()) {
            avaibleSignals = Configurations::uplineTraversing; //我们要怎么遍历
        }
        else {
            QStringList reversedList = Configurations::uplineTraversing;
            std::reverse(reversedList.begin(), reversedList.end());
            avaibleSignals = reversedList;
        }
    }
    if (the_signal_inwhichline == SignalType::UpwardCXSignal) {
        auto the_startsignalpos = signalPositionMap.value(the_first_signal);
        auto the_lastsignalpos = signalPositionMap.value(the_last_signal);
        if (the_startsignalpos.x() > the_lastsignalpos.x()) {
            avaibleSignals = Configurations::uplineTraversingCX; //我们要怎么遍历
        }
        else {
            QStringList reversedList = Configurations::uplineTraversingCX;
            std::reverse(reversedList.begin(), reversedList.end());
            avaibleSignals = reversedList;
        }
    }
    QStringList  route = Utilty::obtainSignalsFromInroutePath(routepath);
    // 找出进路与线路的所有重叠部分
    auto overlap = route.rbegin();
    while ((overlap = std::find_first_of(overlap, route.rend(), avaibleSignals.begin(), avaibleSignals.end())) != route.rend()) {
        // 从线路的这个重叠点开始，向前添加所有的信号机到进路的信号机序列中
        auto lineOverlap = std::find(avaibleSignals.begin(), avaibleSignals.end(), *overlap);
        auto nextOverlap = std::find(lineOverlap, avaibleSignals.end(), *overlap);
        if (nextOverlap == avaibleSignals.end()) {
            nextOverlap = std::find(avaibleSignals.begin(), lineOverlap, *overlap);
        }
        std::copy(lineOverlap, nextOverlap, std::back_inserter(route));
        // 继续查找下一个重叠点
        overlap++;
    }
    return route;
}
//针对性的进行发码
void InterlockControl::FaMaLogicControl(QString routePath) {
    //只有4DG和3-11DG才有可能发检查吗，
    //当4DG的保护信号灯是关闭的时候，它都是发JC码
    //只有当前方第一个分区有车占用的时候，它是发JC码
    //也就说空闲区段和有车占用是不同的两个概念
    //'Unknown', 'Free', 'Occupied'
    //第一步，需要知道运行方向会经过什么区段
    //只能说把含有DG的删除掉
    //第一步。先看运行方向信号灯，确定要遍历哪些区段 （严格意义上，那些LQG都不能算站内区段）
    auto traveringSignals = getForwardSignalsWithSpecificInRoute(routePath);
    auto switchpostionmap = da.getAllTurnoutPositions();
    auto signalPositionmap = da.getAllSignalPositions();
    QMap<QString, QString> segment2fama; //最终轨道应该发什么码
   //第二步，然后，分站内区段，和站内到轨区段，要是运行方向经过带有到轨的就减去1，要是经过站内的，也要减去1
   //判断是要对下行和上行区段都发码，还是只对上行或者下行线路发码
    auto direction = getRouteDirection(routePath, switchpostionmap, signalPositionmap);
    if (direction == RouteDirection::CrossLine) {  //需要对上下线线路都发码
        //今天一定要把这个弄了，我们需要一个算法
        //我们分站外和站内区段来弄
        //我们已经知道哦啊了    
        //不管进路怎么排列，我们都应该首先获取它前面应该会碰到的信号机序列
        auto thesignals_ = Utilty::getTraversingSignalsByFromAndendWithInroutePath(routePath).split("|");
        auto thesegments_ = Utilty::getTraversingSegmentsByFromAndendWithInroutePath(routePath).split("|");
        auto complemtorysgment_ = Utilty::getTraversingCompletorySegmentsByFromAndendWithInroutePath(routePath);
        for (QString t_section : thesegments_) {
            this->m_phydevicemanager->getUpdateStatusForSections();
            auto t_section_status = this->m_phydevicemanager->updateAndFetchTrackSectionStatus(t_section);
            if (t_section_status == "Occupied") {       //如果当前轨道被占用，则发占用检测码
                segment2fama.insert(t_section, "ZY");
                continue;
            }
            int aheadInstationSectionCount = 0;
            int aheadDaoguiSectionCount = 0;
            int aheadFreenSectionCount = 0;
            auto idx = thesegments_.indexOf(t_section);
            auto aheadsignalidx = Utilty::generateAheadCircleNumber(idx, thesignals_.size());
            int howmanysignalsShouldTraverse = 0;
            for (int tmpidx : aheadsignalidx) {
                auto whetherthe_Signal_is_hideen = this->m_trackView->signalStatusMap.value(thesignals_.at(tmpidx));
                if (whetherthe_Signal_is_hideen == SignalStatus::Hidden
                    || whetherthe_Signal_is_hideen == SignalStatus::Open) {
                    howmanysignalsShouldTraverse = howmanysignalsShouldTraverse + 1;
                    continue;
                }
                if (whetherthe_Signal_is_hideen == SignalStatus::Closed) {
                    break;
                }
            }
            if (howmanysignalsShouldTraverse == 0) {
                segment2fama.insert(t_section, "HU");
                continue;
            }
            QStringList mysignalGallery; //要检查的信号机序列
            for (int j = 0; j < howmanysignalsShouldTraverse; j++) {
                mysignalGallery.append(thesignals_.at(aheadsignalidx.indexOf(j)));
            }
            //循环，知道前面有多少到轨区段，有多少站内区段，多少个空闲区段
            for (int i = 0; i < mysignalGallery.size() - 1; i++) {
                QString beginsignal = mysignalGallery[i];
                QString endsignal = mysignalGallery[i + 1];
                QString ztsection = "";
                if (Configurations::instationSignals.contains(beginsignal)
                    && Configurations::instationSignals.contains(endsignal)) {
                    auto tmmp1 = Utilty::getInstationTrackSectionNameWithStartOrEndSignals(beginsignal, endsignal);
                    auto tmmp2 = Utilty::getInstationTrackSectionNameWithStartOrEndSignals(endsignal, beginsignal);
                    if (tmmp1 == "" && tmmp2 != "") {
                        ztsection = tmmp2;
                    }
                    if (tmmp2 == "" && tmmp1 != "") {
                        ztsection = tmmp1;
                    }
                    if (tmmp1 == "" && tmmp2 == "") {
                        break;
                    }
                    if (ztsection.contains("DG") == true) {
                        aheadDaoguiSectionCount++;
                    }
                    else {
                        aheadInstationSectionCount++;
                    }
                    //我们来看一下，这个区段是否被占用
                    auto thestatussec = this->m_phydevicemanager->updateAndFetchTrackSectionStatus(ztsection);
                    if (thestatussec == "Free") {
                        aheadFreenSectionCount++;
                    }
                }
                else {      //站外区段
                    auto tmpsec = da.findSectionIdByStartAndEndSignal(beginsignal, endsignal);
                    auto tmpsec1 = da.findSectionIdByStartAndEndSignal(endsignal, beginsignal);
                    if (tmpsec == "" && tmpsec1 != "") {
                        ztsection = tmpsec1;
                    }
                    if (tmpsec1 == "" && tmpsec != "") {
                        ztsection = tmpsec;
                    }
                    if (tmpsec == "" && tmpsec1 == "") {
                        break;
                    }
                    auto thestatussec = this->m_phydevicemanager->updateAndFetchTrackSectionStatus(ztsection);
                    if (thestatussec == "Free") {
                        aheadFreenSectionCount++;
                    }
                }
            }
            auto the_finalcountahed = aheadFreenSectionCount - aheadInstationSectionCount - aheadDaoguiSectionCount;
            switch (the_finalcountahed)
            {
            case 6:
                segment2fama.insert(t_section, "L3");
            case 5:
                segment2fama.insert(t_section, "L3");
            case 4:
                segment2fama.insert(t_section, "L2");
            case 3:
                segment2fama.insert(t_section, "L");
            case 2:
                segment2fama.insert(t_section, "LU");
            case 1:
                segment2fama.insert(t_section, "U");
            case 0:
                segment2fama.insert(t_section, "HU");
            default:
                segment2fama.insert(t_section, "HU");
                break;
            }
        }
        addedComplementaryFama(complemtorysgment_, segment2fama);
        //将交叉轨道的设置为检查码
        segment2fama.insert("3-11DG", "JC");
        segment2fama.insert("1-9DG", "JC");
        //只发送一个区段的编码
        auto whichlinefama = Utilty::WhatLineShouldFaMa(routePath);
        QMap<QString, QString> finalfama;
        finalfama.clear();
        if (whichlinefama == "Down") {
            finalfama.insert("3-11DG", segment2fama.value("3-11DG"));
            finalfama.insert("IG", segment2fama.value("IG"));
            finalfama.insert("4DG", segment2fama.value("4DG"));
            finalfama.insert("X1LQG", segment2fama.value("X1LQG"));
            finalfama.insert("1101G", segment2fama.value("1101G"));
            finalfama.insert("1103G", segment2fama.value("1103G"));
            finalfama.insert("1105G", segment2fama.value("1105G"));
            finalfama.insert("1107G", segment2fama.value("1107G"));
            finalfama.insert("3G", segment2fama.value("3G"));
        }
        else {
            finalfama.clear();
            finalfama.insert("1-9DG", segment2fama.value("1-9DG"));
            finalfama.insert("IIG", segment2fama.value("IIG"));
            finalfama.insert("2DG", segment2fama.value("2DG"));
            finalfama.insert("S1LQG", segment2fama.value("S1LQG"));
            finalfama.insert("1102G", segment2fama.value("1102G"));
            finalfama.insert("1104G", segment2fama.value("1104G"));
            finalfama.insert("1106G", segment2fama.value("1106G"));
            finalfama.insert("1108G", segment2fama.value("1108G"));
            finalfama.insert("4G", segment2fama.value("4G"));
        }
        auto rectifiedfama = Utilty::rectifiedFaMaInfo(finalfama);
        emit InterlockingFaMA(rectifiedfama.first(), rectifiedfama.last(), whichlinefama);
    }
    else { //只要针对上行或者下行线进行发码
        auto myroutesignals = Utilty::obtainSignalsFromInroutePath(routePath);
        auto startsignal = myroutesignals.first();
        auto endsignal = myroutesignals.last();
        QStringList traverSections; //临时的要遍历的区段
        QStringList finalSectiontraversing;
        QString finalCompleteroySection;        //发码是需要9个区段的，这个是协议限制
        QString finalline;              //Down or Up
        if (Configurations::downlineSignals.contains(startsignal) &&
            Configurations::downlineSignals.contains(endsignal)) {
            if (myroutesignals.contains("S3") || myroutesignals.contains("X3")) {
                traverSections = Configurations::downwardCXTrackSections;
                finalCompleteroySection = "IG";
                finalline = "Down";
            }
            else {
                traverSections = Configurations::downwardTrackSections;
                finalCompleteroySection = "3G";
                finalline = "Down";
            }
            auto startposition = signalPositionmap.value(startsignal);
            auto endposition = signalPositionmap.value(endsignal);
            if (startposition.x() < endposition.x()) {
                finalSectiontraversing = traverSections;
            }
            else {
                std::reverse(traverSections.begin(), traverSections.end());
                finalSectiontraversing = traverSections;
            }
        }
        else { //上行线
            if (myroutesignals.contains("S4") || myroutesignals.contains("X4")) {
                traverSections = Configurations::upwardCXTrackSections;
                finalCompleteroySection = "IIG";
                finalline = "Up";
            }
            else {
                traverSections = Configurations::upwardTrackSections;
                finalCompleteroySection = "4G";
                finalline = "Up";
            }
            auto startposition = signalPositionmap.value(startsignal);
            auto endposition = signalPositionmap.value(endsignal);
            if (startposition.x() > endposition.x()) {
                finalSectiontraversing = traverSections;
            }
            else {
                std::reverse(traverSections.begin(), traverSections.end());
                finalSectiontraversing = traverSections;
            }
        }
        for (QString t_section : finalSectiontraversing) {
            this->m_phydevicemanager->getUpdateStatusForSections();
            auto t_section_status = this->m_phydevicemanager->updateAndFetchTrackSectionStatus(t_section);
            if (t_section_status == "Occupied") {       //如果当前轨道被占用，则发占用检测码
                segment2fama.insert(t_section, "ZY");
                continue;
            }
            else {
                //首先，我们要知道从信号机的哪里遍历到哪里
                //也就说从所要遍历的信号机序列中，我们先找到该区段在序列中的开始和结束信号机对
                //对于站内信号机,没有交叉线路运行的时候是可以这么做的
                QString foundedStartSignal = "";
                QString foundedEndSignal = "";
                for (int i = 0; i < traveringSignals.size(); ++i) {
                    QString current = traveringSignals[i];
                    QString next = traveringSignals[(i + 1) % traveringSignals.size()];
                    if (Configurations::instationSignals.contains(current) && Configurations::instationSignals.contains(next)) {
                        auto the_segmentname1 = Utilty::getInstationTrackSectionNameWithStartOrEndSignals(current, next); //由于没有办法建立合适的数据结构，只能这么弄了
                        auto the_segmentname2 = Utilty::getInstationTrackSectionNameWithStartOrEndSignals(next, current);
                        if (the_segmentname1 == t_section || the_segmentname2 == t_section) {
                            foundedStartSignal = current;
                            foundedEndSignal = next;
                            break;
                        }
                    }
                    else {  //从数据库中寻找
                        auto tmpsec = da.findSectionIdByStartAndEndSignal(current, next);
                        auto tmpsec1 = da.findSectionIdByStartAndEndSignal(next, current);
                        if (tmpsec == t_section || tmpsec1 == t_section) {
                            foundedStartSignal = current;
                            foundedEndSignal = next;
                            break;
                        }
                    }
                }
                //有几个比较特殊的情况需要处理
                //第一种，当轨道区段是到轨的时候，我们需要查看它的起始信号机的状态，也就是保护信号机
                if (t_section.contains("DG") && this->m_trackView->signalStatusMap.value(foundedStartSignal) == SignalStatus::Closed) {
                    segment2fama.insert(t_section, "JC");
                    continue;
                }
                //第二种，当轨道区段不是站内区段，且它的前面一个信号灯的颜色是红白，这表示列车可以越过这个信号机，该区段自然就是HB编码
                if (Configurations::instationSections.contains(t_section) == false
                    && this->m_phydevicemanager->getLastedColorForSignal(foundedEndSignal)->getLight1() == "White" &&
                    this->m_phydevicemanager->getLastedColorForSignal(foundedEndSignal)->getLight2() == "Red") {
                    segment2fama.insert(t_section, "HB");
                    continue;
                }
                int aheadInstationSectionCount = 0;
                int aheadDaoguiSectionCount = 0;
                int aheadFreenSectionCount = 0;
                int idx = traveringSignals.indexOf(foundedEndSignal); //从前一个位置往前看
                auto the_traver_idxs = Utilty::generateAheadCircleNumber(idx, traveringSignals.size());
                int howmanysignalsShouldTraverse = 0;
                for (int tmpidx : the_traver_idxs) {
                    auto whetherthe_Signal_is_hideen = this->m_trackView->signalStatusMap.value(traveringSignals.at(tmpidx));
                    if (whetherthe_Signal_is_hideen == SignalStatus::Hidden
                        || whetherthe_Signal_is_hideen == SignalStatus::Open) {
                        howmanysignalsShouldTraverse = howmanysignalsShouldTraverse + 1;
                        continue;
                    }
                    if (whetherthe_Signal_is_hideen == SignalStatus::Closed) {
                        break;
                    }
                }
                if (howmanysignalsShouldTraverse == 0) {
                    segment2fama.insert(t_section, "HU");
                    continue;
                }
                QStringList mysignalGallery; //要检查的信号机序列
                mysignalGallery.append(foundedEndSignal);
                for (int j = 0; j < howmanysignalsShouldTraverse; j++) {
                    mysignalGallery.append(traveringSignals.at(the_traver_idxs.indexOf(j)));
                }
                //循环，知道前面有多少到轨区段，有多少站内区段，多少个空闲区段
                for (int i = 0; i < mysignalGallery.size() - 1; i++) {
                    QString beginsignal = mysignalGallery[i];
                    QString endsignal = mysignalGallery[i + 1];
                    QString ztsection = "";
                    if (Configurations::instationSignals.contains(beginsignal)
                        && Configurations::instationSignals.contains(endsignal)) {
                        auto tmmp1 = Utilty::getInstationTrackSectionNameWithStartOrEndSignals(beginsignal, endsignal);
                        auto tmmp2 = Utilty::getInstationTrackSectionNameWithStartOrEndSignals(endsignal, beginsignal);
                        if (tmmp1 == "" && tmmp2 != "") {
                            ztsection = tmmp2;
                        }
                        if (tmmp2 == "" && tmmp1 != "") {
                            ztsection = tmmp1;
                        }
                        if (ztsection.contains("DG") == true) {
                            aheadDaoguiSectionCount++;
                        }
                        else {
                            aheadInstationSectionCount++;
                        }
                        //我们来看一下，这个区段是否被占用
                        auto thestatussec = this->m_phydevicemanager->updateAndFetchTrackSectionStatus(ztsection);
                        if (thestatussec == "Free") {
                            aheadFreenSectionCount++;
                        }
                    }
                    else {      //站外区段
                        auto tmpsec = da.findSectionIdByStartAndEndSignal(beginsignal, endsignal);
                        auto tmpsec1 = da.findSectionIdByStartAndEndSignal(endsignal, beginsignal);
                        if (tmpsec == "" && tmpsec1 != "") {
                            ztsection = tmpsec1;
                        }
                        if (tmpsec1 == "" && tmpsec != "") {
                            ztsection = tmpsec;
                        }
                        auto thestatussec = this->m_phydevicemanager->updateAndFetchTrackSectionStatus(ztsection);
                        if (thestatussec == "Free") {
                            aheadFreenSectionCount++;
                        }
                    }
                }
                auto the_finalcountahed = aheadFreenSectionCount - aheadInstationSectionCount - aheadDaoguiSectionCount;
                switch (the_finalcountahed)
                {
                case 6:
                    segment2fama.insert(t_section, "L3");
                case 5:
                    segment2fama.insert(t_section, "L3");
                case 4:
                    segment2fama.insert(t_section, "L2");
                case 3:
                    segment2fama.insert(t_section, "L");
                case 2:
                    segment2fama.insert(t_section, "LU");
                case 1:
                    segment2fama.insert(t_section, "U");
                case 0:
                    segment2fama.insert(t_section, "HU");
                default:
                    segment2fama.insert(t_section, "HU");
                    break;
                }
            }
        }
        this->m_phydevicemanager->getUpdateStatusForSections();
        auto t_section_status1 = this->m_phydevicemanager->updateAndFetchTrackSectionStatus(finalCompleteroySection);
        if (t_section_status1 == "Occupied") {       //如果当前轨道被占用，则发占用检测码
            segment2fama.insert(finalCompleteroySection, "ZY");
            //在这里进行发码
        }
        //其实还是有一个发码区段要注意的，发码区段有9个，但是遍历的时候，有可能只有8个
        if (finalCompleteroySection == "3G" || finalCompleteroySection == "IG") {
            //找到当前的
            auto t_trackstatus = da.getCurrentTrackDirectionStatus("Down");
            QString direction;
            QString signaltobewatched;
            //就是看运行方向前一个信号机的状态
            if (t_trackstatus == "Forward") {
                signaltobewatched = "X" + finalCompleteroySection.replace("G", "");
                signaltobewatched = signaltobewatched.trimmed();
            }
            else {
                signaltobewatched = "S" + finalCompleteroySection.replace("G", "");
                signaltobewatched = signaltobewatched.trimmed();
            }
            //
            auto signalstaus = this->m_trackView->signalStatusMap.value(signaltobewatched);
            if (signalstaus == SignalStatus::Closed) {
                segment2fama.insert(finalCompleteroySection, "HU");
            }
            else {
                if (finalCompleteroySection == "3G" && t_trackstatus == "Forward") {
                    auto the_segaheadstauts = segment2fama.value("4DG");
                    if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                        segment2fama.insert(finalCompleteroySection, "HU");
                    }
                    else {
                        segment2fama.insert(finalCompleteroySection, "UU");
                    }
                }
                if (finalCompleteroySection == "3G" && t_trackstatus == "Reverse") {
                    auto the_segaheadstauts = segment2fama.value("3-11DG");
                    if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                        segment2fama.insert(finalCompleteroySection, "HU");
                    }
                    else {
                        segment2fama.insert(finalCompleteroySection, "UU");
                    }
                }
                if (finalCompleteroySection == "IG" && t_trackstatus == "Forward") {
                    auto the_segaheadstauts = segment2fama.value("4DG");
                    if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                        segment2fama.insert(finalCompleteroySection, "HU");
                    }
                    else {
                        //前面这个码加1
                        segment2fama.insert(finalCompleteroySection, Utilty::calculateFaMaAdvancedOneStep(the_segaheadstauts));
                    }
                }
                if (finalCompleteroySection == "IG" && t_trackstatus == "Reverse") {
                    auto the_segaheadstauts = segment2fama.value("3-11DG");
                    if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                        segment2fama.insert(finalCompleteroySection, "HU");
                    }
                    else {
                        //前面这个码加1
                        segment2fama.insert(finalCompleteroySection, Utilty::calculateFaMaAdvancedOneStep(the_segaheadstauts));
                    }
                }
            }
        }
        if (finalCompleteroySection == "4G" || finalCompleteroySection == "IIG") {
            //如果是占用，那就是占用
            //找到当前的
            auto t_trackstatus = da.getCurrentTrackDirectionStatus("Up");
            QString direction;
            QString signaltobewatched;
            //就是看运行方向前一个信号机的状态
            if (t_trackstatus == "Forward") {
                signaltobewatched = "S" + finalCompleteroySection.replace("G", "");
                signaltobewatched = signaltobewatched.trimmed();
            }
            else {
                signaltobewatched = "X" + finalCompleteroySection.replace("G", "");
                signaltobewatched = signaltobewatched.trimmed();
            }
            //
            auto signalstaus = this->m_trackView->signalStatusMap.value(signaltobewatched);
            if (signalstaus == SignalStatus::Closed) {
                segment2fama.insert(finalCompleteroySection, "HU");
            }
            else {
                if (finalCompleteroySection == "4G" && t_trackstatus == "Forward") {
                    auto the_segaheadstauts = segment2fama.value("1-9DG");
                    if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                        segment2fama.insert(finalCompleteroySection, "HU");
                    }
                    else {
                        segment2fama.insert(finalCompleteroySection, "UU");
                    }
                }
                if (finalCompleteroySection == "4G" && t_trackstatus == "Reverse") {
                    auto the_segaheadstauts = segment2fama.value("2DG");
                    if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                        segment2fama.insert(finalCompleteroySection, "HU");
                    }
                    else {
                        segment2fama.insert(finalCompleteroySection, "UU");
                    }
                }
                if (finalCompleteroySection == "IIG" && t_trackstatus == "Forward") {
                    auto the_segaheadstauts = segment2fama.value("1-9DG");
                    if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                        segment2fama.insert(finalCompleteroySection, "HU");
                    }
                    else {
                        //前面这个码加1
                        segment2fama.insert(finalCompleteroySection, Utilty::calculateFaMaAdvancedOneStep(the_segaheadstauts));
                    }
                }
                if (finalCompleteroySection == "IIG" && t_trackstatus == "Reverse") {
                    auto the_segaheadstauts = segment2fama.value("2DG");
                    if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                        segment2fama.insert(finalCompleteroySection, "HU");
                    }
                    else {
                        //前面这个码加1
                        segment2fama.insert(finalCompleteroySection, Utilty::calculateFaMaAdvancedOneStep(the_segaheadstauts));
                    }
                }
            }
        }
        //最后统一发码出去
        auto rectifiedfama = Utilty::rectifiedFaMaInfo(segment2fama);
        emit InterlockingFaMA(rectifiedfama.first(), rectifiedfama.last(), finalline);
    }
}
void InterlockControl::addedComplementaryFama(QString finalCompleteroySection, QMap<QString, QString>& segment2fama) {
    //其实还是有一个发码区段要注意的，发码区段有9个，但是遍历的时候，有可能只有8个
    if (finalCompleteroySection == "3G" || finalCompleteroySection == "IG") {
        //找到当前的
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Down");
        QString direction;
        QString signaltobewatched;
        //就是看运行方向前一个信号机的状态
        if (t_trackstatus == "Forward") {
            signaltobewatched = "X" + finalCompleteroySection.replace("G", "");
            signaltobewatched = signaltobewatched.trimmed();
        }
        else {
            signaltobewatched = "S" + finalCompleteroySection.replace("G", "");
            signaltobewatched = signaltobewatched.trimmed();
        }
        //
        auto signalstaus = this->m_trackView->signalStatusMap.value(signaltobewatched);
        if (signalstaus == SignalStatus::Closed) {
            segment2fama.insert(finalCompleteroySection, "HU");
        }
        else {
            if (finalCompleteroySection == "3G" && t_trackstatus == "Forward") {
                auto the_segaheadstauts = segment2fama.value("4DG");
                if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                    segment2fama.insert(finalCompleteroySection, "HU");
                }
                else {
                    segment2fama.insert(finalCompleteroySection, "UU");
                }
            }
            if (finalCompleteroySection == "3G" && t_trackstatus == "Reverse") {
                auto the_segaheadstauts = segment2fama.value("3-11DG");
                if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                    segment2fama.insert(finalCompleteroySection, "HU");
                }
                else {
                    segment2fama.insert(finalCompleteroySection, "UU");
                }
            }
            if (finalCompleteroySection == "IG" && t_trackstatus == "Forward") {
                auto the_segaheadstauts = segment2fama.value("4DG");
                if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                    segment2fama.insert(finalCompleteroySection, "HU");
                }
                else {
                    //前面这个码加1
                    segment2fama.insert(finalCompleteroySection, Utilty::calculateFaMaAdvancedOneStep(the_segaheadstauts));
                }
            }
            if (finalCompleteroySection == "IG" && t_trackstatus == "Reverse") {
                auto the_segaheadstauts = segment2fama.value("3-11DG");
                if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                    segment2fama.insert(finalCompleteroySection, "HU");
                }
                else {
                    //前面这个码加1
                    segment2fama.insert(finalCompleteroySection, Utilty::calculateFaMaAdvancedOneStep(the_segaheadstauts));
                }
            }
        }
    }
    if (finalCompleteroySection == "4G" || finalCompleteroySection == "IIG") {
        //如果是占用，那就是占用
        //找到当前的
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Up");
        QString direction;
        QString signaltobewatched;
        //就是看运行方向前一个信号机的状态
        if (t_trackstatus == "Forward") {
            signaltobewatched = "S" + finalCompleteroySection.replace("G", "");
            signaltobewatched = signaltobewatched.trimmed();
        }
        else {
            signaltobewatched = "X" + finalCompleteroySection.replace("G", "");
            signaltobewatched = signaltobewatched.trimmed();
        }
        //
        auto signalstaus = this->m_trackView->signalStatusMap.value(signaltobewatched);
        if (signalstaus == SignalStatus::Closed) {
            segment2fama.insert(finalCompleteroySection, "HU");
        }
        else {
            if (finalCompleteroySection == "4G" && t_trackstatus == "Forward") {
                auto the_segaheadstauts = segment2fama.value("1-9DG");
                if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                    segment2fama.insert(finalCompleteroySection, "HU");
                }
                else {
                    segment2fama.insert(finalCompleteroySection, "UU");
                }
            }
            if (finalCompleteroySection == "4G" && t_trackstatus == "Reverse") {
                auto the_segaheadstauts = segment2fama.value("2DG");
                if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                    segment2fama.insert(finalCompleteroySection, "HU");
                }
                else {
                    segment2fama.insert(finalCompleteroySection, "UU");
                }
            }
            if (finalCompleteroySection == "IIG" && t_trackstatus == "Forward") {
                auto the_segaheadstauts = segment2fama.value("1-9DG");
                if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                    segment2fama.insert(finalCompleteroySection, "HU");
                }
                else {
                    //前面这个码加1
                    segment2fama.insert(finalCompleteroySection, Utilty::calculateFaMaAdvancedOneStep(the_segaheadstauts));
                }
            }
            if (finalCompleteroySection == "IIG" && t_trackstatus == "Reverse") {
                auto the_segaheadstauts = segment2fama.value("2DG");
                if (the_segaheadstauts == "JC" || the_segaheadstauts == "ZY") {
                    segment2fama.insert(finalCompleteroySection, "HU");
                }
                else {
                    //前面这个码加1
                    segment2fama.insert(finalCompleteroySection, Utilty::calculateFaMaAdvancedOneStep(the_segaheadstauts));
                }
            }
        }
    }
}
void InterlockControl::handlerSwitchDingcaoOperationRequest(QString switchName) {
    qDebug() << "ding cao...................." << switchName;
    bool ok;
    int switch_Name = switchName.toInt(&ok);
    if (ok) {
        // 转换成功
        qDebug() << "Converted value:" << switch_Name;
    }
    else {
        // 转换失败
        qDebug() << "Failed to convert to int";
    }
    auto the_segment_name = da.getTrackSectionIdById(switch_Name);
    qDebug() << "we get segmentname..." << the_segment_name;
    //首先第一步判断，是否对应的区段有车占用
    auto the_sgcolor = this->m_trackView->m_lineMap.value(the_segment_name);
    auto the_color = the_sgcolor->getColor();
    qDebug() << "场景中的区段颜色是..." << the_color;
    if (the_color == "Red" || the_color == "White") {
        return;         //不允许改变方向
    }
    //发出信号通知TrackCommander发出消息
    emit switchDirectOperationRequested(switchName);
    //手动触发状态机
    // 切换到 m_checkingState 状态
    QCoreApplication::postEvent(m_stateMachine_switchDC, new SwitchDirectOperationEvent());
    return;
}
void InterlockControl::handlerSwitchReverseOperationRequest(QString swithcName) {
    qDebug() << "fan cao...................." << swithcName;
    bool ok;
    int switch_Name = swithcName.toInt(&ok);
    if (ok) {
        // 转换成功
        qDebug() << "Converted value:" << switch_Name;
    }
    else {
        // 转换失败
        qDebug() << "Failed to convert to int";
    }
    auto the_segment_name = da.getTrackSectionIdById(switch_Name);
    qDebug() << "we get segmentname..." << the_segment_name;
    //首先第一步判断，是否对应的区段有车占用
    auto the_sgcolor = this->m_trackView->m_lineMap.value(the_segment_name);
    auto the_color = the_sgcolor->getColor();
    qDebug() << "场景中的区段颜色是..." << the_color;
    if (the_color == "Red" || the_color == "White") {
        return;         //不允许改变方向
    }
    //发出信号通知TrackCommander发出消息
    emit switchReverseOperationRequested(swithcName);
    QCoreApplication::postEvent(m_stateMachine_switchFC, new SwitchReverseOperationEvent());
}
//; //处理驱动物理信号机颜色变化
void  InterlockControl::handlerDrivePhySignalStatusRequest()
{
    qDebug() << "MBBBBBBB....";
    auto alloutsignal = Configurations::outstationSignals;
    auto allinsignal = Configurations::instationSignals;
    auto allsigna = alloutsignal + allinsignal;
    QMap<QString, QString> tmp;
    for (QString the_sig : allsigna) {
        tmp.insert(the_sig, this->m_trackView->getSignalColorByName(the_sig));
        qDebug() << "MB...." << the_sig << this->m_trackView->getSignalColorByName(the_sig);
    }
    emit notifyTrackCommanderForDrivingSignalChange(tmp);
    return;
}
void InterlockControl::handlerSignalStatusCheckandPossibleFama(QMap<QString, QString> sig2color) {
    //检查所有信号机是否和界面中状态一致
    bool allsignalconsistent = true;
    auto allsigal = Configurations::instationSignals + Configurations::outstationSignals;
    for (QString the_sig : allsigal) {
        auto the_sg_color = sig2color.value(the_sig + "-B1") + sig2color.value(the_sig + "-B2");
        auto current_color = this->m_phydevicemanager->getPhySignalColorStatusById(the_sig);
        if (the_sg_color != current_color) {
            allsignalconsistent = false;
            break;
        }
    }
    if (allsignalconsistent == true) {
        //发出可以开始发码信号
        //考虑发出四种信号
        //有轨道被占用，有进路
        //有轨道被占用，无进路
        //无轨道被占用，有进路
        //无轨道被占用，无进路
    }
    return;
}
void InterlockControl::handleSwitchDirectOperationSuccess(QString switchId, QString commandId)
{
    // 切换到 m_successState 状态
    emit switchDirectOperationSucceeded();
    emit drawSwitchDirectedInScene(switchId);
}
void InterlockControl::handleSwitchDirectOperationFailure(QString switchId, QString commandId)
{
    // 切换到 m_checkingState 状态
    emit switchDirectOperationFailed();
}
void InterlockControl::handleSwitchReverseOperationSuccess(QString switchId, QString commandId)
{
    // 切换到 m_successState 状态
    emit switchReverseOperationSucceeded();
    // emit drawSwitchDirectionSegment(switchId);  //这个地方要修改
    emit drawSwitchReverseInScene(switchId);
}
void InterlockControl::handleSwitchReverseOperationFailure(QString switchId, QString commandId)
{
    // 切换到 m_checkingState 状态
    emit switchReverseOperationFailed();
}
void InterlockControl::findAdjacentStrings(const QString& input, const QString& target, QString& prev, QString& next)
{
    QStringList tokens = input.split(" -> ");
    int index = tokens.indexOf(target);
    if (index != -1) {
        if (index > 0) {
            prev = tokens[index - 1];
        }
        if (index < tokens.size() - 1) {
            next = tokens[index + 1];
        }
    }
}
double InterlockControl::calculateAngleBetweenLineSegments(const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPointF& p4)
{
    // 计算第一条线段的角度
    double angle1 = std::atan2(p2.y() - p1.y(), p2.x() - p1.x()) * 180.0 / M_PI;
    // 计算第二条线段的角度
    double angle2 = std::atan2(p4.y() - p3.y(), p4.x() - p3.x()) * 180.0 / M_PI;
    // 计算两条线段之间的夹角
    double angle = std::abs(angle2 - angle1);
    // 如果夹角大于 180 度,则需要从 360 度中减去该值
    if (angle > 180.0) {
        angle = 360.0 - angle;
    }
    return angle;
}
TrackDirection InterlockControl::getLineDirectionFromPath(const QString& path)
{
    QStringList pathElements = path.split(" -> ");
    QSet<QString> downlineSignals = { "X", "SI", "XI", "S3", "X3", "SN" };
    QSet<QString> uplineSignals = { "XN", "SII", "XII", "S4", "X4", "S" };
    bool hasDownlineSection = false;
    bool hasUplineSection = false;
    for (const QString& element : pathElements)
    {
        if (downlineSignals.contains(element.trimmed()))
        {
            hasDownlineSection = true;
        }
        else if (uplineSignals.contains(element.trimmed()))
        {
            hasUplineSection = true;
        }
    }
    if (hasDownlineSection && !hasUplineSection)
    {
        return TrackDirection::Downward;
    }
    else if (!hasDownlineSection && hasUplineSection)
    {
        return TrackDirection::Upward;
    }
    else
    {
        return TrackDirection::Unknown;
    }
}
//假设我们知道进路在下行线运行，则依靠这个函数我们知道进路是正向还是反向
RouteDirection InterlockControl::getRouteDirectionForDownlinePath(const QString& path, const QMap<QString, QPointF>& switchPositionMap, const QMap<QString, QPointF>& signalPositionMap)
{
    QStringList pathElements = path.split(" -> ");
    QList<QPointF> positions;
    for (const QString& element : pathElements)
    {
        auto t_element = element.trimmed();
        bool ok;
        t_element.toInt(&ok);
        if (ok)
        {
            // 转辙机
            if (switchPositionMap.contains(t_element))
            {
                positions.append(switchPositionMap[t_element]);
            }
        }
        else
        {
            // 信号机
            if (signalPositionMap.contains(t_element))
            {
                positions.append(signalPositionMap[t_element]);
            }
        }
    }
    bool xIncreasing = true;
    bool yDecreasing = true;
    for (int i = 0; i < positions.size() - 1; i++)
    {
        const QPointF& curr = positions[i];
        const QPointF& next = positions[i + 1];
        if (next.x() > curr.x() && (next.y() <= curr.y() || qFuzzyCompare(next.y(), curr.y())))
        {
            // 下行正向
        }
        else if (next.x() < curr.x() && (next.y() <= curr.y() || qFuzzyCompare(next.y(), curr.y())))
        {
            // 上行反向
            xIncreasing = false;
            yDecreasing = true;
            break;
        }
        else
        {
            xIncreasing = false;
            yDecreasing = false;
            break;
        }
    }
    if (xIncreasing && yDecreasing)
    {
        return RouteDirection::DownwardForward;
    }
    else
    {
        return RouteDirection::DownwardReverse;
    }
}
RouteDirection InterlockControl::getRouteDirectionForUplinePath(const QString& path, const QMap<QString, QPointF>& switchPositionMap, const QMap<QString, QPointF>& signalPositionMap)
{
    QStringList pathElements = path.split(" -> ");
    QList<QPointF> positions;
    for (const QString& element : pathElements)
    {
        auto t_element = element.trimmed();
        bool ok;
        t_element.toInt(&ok);
        if (ok)
        {
            // 转辙机
            if (switchPositionMap.contains(t_element))
            {
                positions.append(switchPositionMap[t_element]);
            }
        }
        else
        {
            // 信号机
            if (signalPositionMap.contains(t_element))
            {
                positions.append(signalPositionMap[t_element]);
            }
        }
    }
    bool xIncreasing = false;
    bool yDecreasing = true;
    for (int i = 0; i < positions.size() - 1; i++)
    {
        const QPointF& curr = positions[i];
        const QPointF& next = positions[i + 1];
        if (next.x() < curr.x() && (next.y() <= curr.y() || qFuzzyCompare(next.y(), curr.y())))
        {
            // 上行反向
        }
        else if (next.x() > curr.x() && (next.y() <= curr.y() || qFuzzyCompare(next.y(), curr.y())))
        {
            // 下行正向
            xIncreasing = true;
            yDecreasing = false;
            break;
        }
        else
        {
            xIncreasing = false;
            yDecreasing = false;
            break;
        }
    }
    if (xIncreasing && yDecreasing)
    {
        return RouteDirection::UpwardForward;
    }
    else
    {
        return RouteDirection::UpwardReverse;
    }
}



void InterlockControl::handlerSegmmentTakenEvent(QString segmentName) {
    //其实还是一样的分站外和站内区段
    bool isinstationsection = Configurations::instationSections.contains(segmentName);
    emit notifyViewControllerSectionColorShoudChange(segmentName, "Red");

    if (isinstationsection == false) {//站外区段
        auto whichline = Utilty::WhatLineTheSectionIsIn(segmentName);
        auto t_tracklinedirection = da.getCurrentTrackDirectionStatus(whichline);
        auto the_startsx = da.getStartSignalId(segmentName);
        auto the_endsx = da.getEndSignalId(segmentName);
        QString the_protectedSignal = (t_tracklinedirection == "Forward") ? the_startsx : the_endsx;
        this->m_trackView->SetSignalStatusToClose(the_protectedSignal);
        if (t_tracklinedirection == "Forward") {
            this->m_trackView->setSignalLightColorInScene(the_protectedSignal, "RedBlack");
            emit driverSignalColorInShapan(the_protectedSignal, "RedBlack");
        }
        else {
            this->m_trackView->setSignalLightColorInScene(the_protectedSignal, "BlackBlack");
            emit driverSignalColorInShapan(the_protectedSignal, "BlackBlack");
        }
        return;
    }
    else {
        //如果是站内区段
        auto whichline = Utilty::WhatLineTheSectionIsIn(segmentName);
        auto t_tracklinedirection = da.getCurrentTrackDirectionStatus(whichline);
        auto the_startsx = da.getStartSignalId(segmentName);
        auto the_endsx = da.getEndSignalId(segmentName);
        QString the_protectedSignal = (t_tracklinedirection == "Forward") ? the_startsx : the_endsx;
        this->m_trackView->SetSignalStatusToClose(the_protectedSignal);
        this->m_trackView->setSignalLightColorInScene(the_protectedSignal, "RedBlack");
        emit driverSignalColorInShapan(the_protectedSignal, "RedBlack");
        this->m_trackView->SetSignalStatusToClose(the_protectedSignal);
        auto ids = da.getUniqueTurnoutIdsByTrackSectionId(segmentName);
        for (QString the_turnout_id_ : ids) {
            auto the_switch_status_ = da.getTurnoutStatusById(the_turnout_id_);
            if (the_switch_status_ == "Normal") {
                emit drawSwitchDirectedInScene(the_turnout_id_);
            }
            else if (the_switch_status_ == "Reverse") {
                emit drawSwitchReverseInScene(the_turnout_id_);
            }
            else {
                emit drawSwitchLostInScene(the_turnout_id_);

            }
        }
        //一样让保护信号机关闭，且置为红灯
        
    }


}



//首先
void InterlockControl::handlerSegmentClearEvent(QString segmentName) {


    bool isinstationsection = !Configurations::instationSections.contains(segmentName);

    if (!isinstationsection) {
        emit notifyViewControllerSectionColorShoudChange(segmentName, "Default");
        if (Configurations::downlineOutstationSgments.contains(segmentName)) {
            handleOutstationSegment(segmentName, "Down");
        }
        else {
            handleOutstationSegment(segmentName, "Up");
        }
        return;
    }

    //如果是站内区段
    //同样告诉viewController改变线段的颜色，通知其修改该区段里面可能存在的道岔，并针对这些道岔进行重新绘制
    //判断是否有进路包含这个区段，如果包含这个区段的，看包含这个区段的进路的的所有其它区段都是否是默认状态，如果是则通知清除进路
    //最后，查看所关联的信号机，并记住要设置其状态，到底是开放，关闭，还是隐藏
    emit notifyViewControllerSectionColorShoudChange(segmentName, "Default");
    auto ids = da.getUniqueTurnoutIdsByTrackSectionId(segmentName);
    for (QString the_turnout_id_ : ids) {
        auto the_switch_status_ = da.getTurnoutStatusById(the_turnout_id_);
        if (the_switch_status_ == "Normal") {
            emit drawSwitchDirectedInScene(the_turnout_id_);
        }else if(the_switch_status_ == "Reverse"){
            emit drawSwitchReverseInScene(the_turnout_id_);
        }
        else {
            emit drawSwitchLostInScene(the_turnout_id_);

        }
    }

    //遍历进路表中的进路
    auto allvalidInterlockingRoute = da.getAllValidRoutes();
    for (QString the_route_path_ : allvalidInterlockingRoute) {
        auto the_segments = getTrackSectionTakenByRoutepath(the_route_path_);
        //判断所有这些区段是否已经出清，也就是到达默认状态
        bool allsegmentclear = true;
        for (QString the_segment_name : the_segments) {
            auto the_sgcolor = this->m_trackView->m_lineMap.value(the_segment_name);
            auto the_color = the_sgcolor->getColor();
            if (the_color != "Default") {
                break;
            }
        }

        if (allsegmentclear == true) {
            //先对数据库进行操作，社会这个进路表中的对应进路是无效的，然后通知ViewLogicalControl重新绘制进路所对应区段
            auto id = da.getInterLockingRouteIDByRoutePath(the_route_path_);
            da.updateRouteValidity(id, false);
            emit notifyViewControllerRemoveInroutePath(the_route_path_, the_segments); //注意在清除进路的时候，
        }
    }
    //当被清除的时候，还是要看保护信号灯的颜色应该是什么
    auto whatlinethesegmentisin = Utilty::WhatLineTheSectionIsIn(segmentName);
    auto t_tracklinedirection = da.getCurrentTrackDirectionStatus(whatlinethesegmentisin);  //是正向还是反向
    auto the_startsx = da.getStartSignalId(segmentName);
    auto the_endsx = da.getEndSignalId(segmentName);
    QString the_protectedSignal = (t_tracklinedirection == "Forward") ? the_startsx : the_endsx;
    emit notifyViewControllerChangeSignalColorAndStatusCarefullyWhenSegmeneClear(the_protectedSignal, segmentName); //这里先不搞。麻痹的快累死了

    return;
}


void InterlockControl::handleOutstationSegment(QString segmentName, QString lineDirection) {
    auto t_tracklinedirection = da.getCurrentTrackDirectionStatus(lineDirection);
    auto the_startsx = da.getStartSignalId(segmentName);
    auto the_endsx = da.getEndSignalId(segmentName);
    QString the_protectedSignal = (t_tracklinedirection == "Forward") ? the_startsx : the_endsx;

    if (!Configurations::instationSignals.contains(the_protectedSignal)) {
        this->m_trackView->SetSignalStatusToOpend(the_protectedSignal);
        if (t_tracklinedirection == "Forward") {
            auto the_aheadsignal = this->m_trackView->getSignalColorByName(the_endsx);
            QString color;
            if (the_aheadsignal.contains("Red")) {
                color = "YellowBlack";
            }
            else if (the_aheadsignal.contains("YellowBlack")) {
                color = "GreenYellow";
            }
            else {
                color = "GreenBlack";
            }
            this->m_trackView->setSignalLightColorInScene(the_protectedSignal, color);
            emit driverSignalColorInShapan(the_protectedSignal, color);
        }
        else {
            this->m_trackView->setSignalLightColorInScene(the_protectedSignal, "BlackBlack");
            emit driverSignalColorInShapan(the_protectedSignal, "BlackBlack");
        }
    }
}
