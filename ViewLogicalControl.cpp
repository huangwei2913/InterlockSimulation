#include "ViewLogicalControl.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector2D>
#include "Configurations.h"
#include "Utilty.h"
#include <algorithm>
ViewLogicalControl::ViewLogicalControl(QObject* parent)
    : QObject(parent)
{
    //联锁转辙机声明
    consolidateSwitchs.insert("1", "3");
    consolidateSwitchs.insert("3", "1");
    consolidateSwitchs.insert("5", "7");
    consolidateSwitchs.insert("7", "5");
    this->switchlockedMap.insert("1", false);
    this->switchlockedMap.insert("2", false);
    this->switchlockedMap.insert("3", false);
    this->switchlockedMap.insert("5", false);
    this->switchlockedMap.insert("4", false);
    this->switchlockedMap.insert("7", false);
    this->switchlockedMap.insert("9", false);
    this->switchlockedMap.insert("11", false);
    switchPositionMap.clear();
}
ViewLogicalControl::~ViewLogicalControl()
{
}
void ViewLogicalControl::handlerSwitchLockingRequet(QStringList siwthcss) {
    for (const QString& key : siwthcss) {
        if (switchlockedMap.contains(key)) {
            switchlockedMap[key] = true;
        }
    }
}
void ViewLogicalControl::drawPath(const QString& pathString, QColor color) {


    reloadSwitchPositionMap();
    reloadSignalPositionMap();
    QStringList pathElements = pathString.split(" -> ");
    for (int i = 0; i < pathElements.size(); i++)
    {
        pathElements[i] = pathElements[i].trimmed();
    }
    // 遍历每个元素,并绘制线段
    for (int i = 0; i < pathElements.size() - 1; i++)
    {
        QString from = pathElements[i];
        QString to = pathElements[i + 1];
        QPointF fromPos, toPos;
        bool fromIsNumber = false, toIsNumber = false;
        // 判断 from 是否为纯数字
        bool ok;
        from.toInt(&ok);
        if (ok)
        {
            fromPos = switchPositionMap[from];

            fromIsNumber = true;
        }
        else
        {
            fromPos = signalPositionMap.value(from);
        }
        // 判断 to 是否为纯数字
        to.toInt(&ok);
        if (ok)
        {

            toPos = switchPositionMap[to];


            toIsNumber = true;
        }
        else
        {
            toPos = signalPositionMap.value(to);
        }
        // 使用  类绘制线段
        QString pathName = from + "->" + to;
        this->m_trackView->drawPath(pathName, fromPos, toPos, color);
        this->Line2CorlorMap.insert(pathName, color);
    }

}


//移除某个routepath的白光带,参照经过的区段
void ViewLogicalControl::handlerInoutePathClearRequest(QString routepath, QStringList segments) {

    //第一步：删除所有区段经过的道岔的
    for (QString the_sgment : segments) {
        auto myturnouts = da.getUniqueTurnoutIdsByTrackSectionId(the_sgment);
        for (QString the_route_id : myturnouts) {
            removeLinesFromSwitch(the_route_id);
        }
    }
    //第二步：按照路径来绘制，让他们变成默认的颜色
    QColor m_color;
    m_color.setRed(77);
    m_color.setGreen(118);
    m_color.setBlue(179);
    drawPath(routepath, m_color);

    //第三步：需要重新绘制区段经过的道岔

    auto thejson = da.getRouteDescription(routepath.split(" -> ").first().trimmed(), routepath.split(" -> ").last().trimmed());

    QJsonDocument jsonDocument = QJsonDocument::fromJson(thejson.toUtf8());
    QJsonObject jsonObject = jsonDocument.object();

    // 将 QJsonObject 转换为 QMap
    QMap<QString, QString> tempMap;
    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
        tempMap.insert(it.key(), it.value().toString());
        auto the_direction = it.value();
        if (the_direction == "Normal") {
            handlerSwitchDirectedRequest(it.key());
        }
        else if (the_direction == "Reverse") {
            handlerSwitchReverseRequest(it.key());
        }
        else {
            handlerSwitchLostRequest(it.key());
        }
    }

}



void ViewLogicalControl::handlerInroutePathReuest(QString routepath) {
    drawPath(routepath, Qt::white);
    //接着，我们应该画区段的颜色
    QStringList temp;
    QStringList pathElements = routepath.split(" -> ");
    for (int i = 0; i < pathElements.size(); i++)
    {
        pathElements[i] = pathElements[i].trimmed();
    }
    auto matchingroute = da.findMatchingRoute(pathElements.first(), pathElements.last());
    auto sections = matchingroute.last();
    //接着寻找
    QRegularExpression re("'(.*?)'");
    QRegularExpressionMatchIterator matchIterator = re.globalMatch(sections);
    QString lastsection;
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        QString trackSection = match.captured(1); // 获取匹配的轨道区段名称
        qDebug() << trackSection;
        lastsection = trackSection;
    }
    auto startendsignslpair = da.getStartEndSignals(lastsection);
    QString startsignal = startendsignslpair.first;
    QString endsignal = startendsignslpair.second;
    this->m_trackView->drawPathBetweenSignals(startsignal, endsignal, Qt::white);
    this->Line2CorlorMap.insert(startsignal + "->" + endsignal, Qt::white);
    //此外，我们要修改转辙机显示的颜色
    auto jsonlizedDes = da.getRouteDescription(startsignal, endsignal);
    drawSwitchsForInroutePath(routepath, jsonlizedDes);
    //此外,我们需要改变道岔文本显示的颜色
    changeTurnoutTextColor(routepath, jsonlizedDes);
    //查看进路的开始和结束信号机是否有相似的
    QStringList pathElementssss = routepath.split(" -> ");
    QString startSignalId = pathElementssss.first().trimmed();
    QString endSignalId = pathElementssss.last().trimmed();
    if (startSignalId.at(0) == endSignalId.at(0))
    {
        this->m_trackView->signalStatusMap.insert(startSignalId, SignalStatus::Open);
        this->m_trackView->signalStatusMap.insert(startSignalId, SignalStatus::Closed);
    }
    else
    {
        // 开始信号机和结束信号机不相似
        this->m_trackView->signalStatusMap.insert(startSignalId, SignalStatus::Open);
        this->m_trackView->signalStatusMap.insert(startSignalId, SignalStatus::Hidden);
    }
    emit InRouteArranged(routepath);  //已经安排了一个进路了
}
void ViewLogicalControl::changeTurnoutTextColor(const QString& routePath, const QString& todirectionJson)
{
    // 分割路径字符串并去除首尾空格
    QStringList pathElements = routePath.split(" -> ");
    for (int i = 0; i < pathElements.size(); i++)
    {
        pathElements[i] = pathElements[i].trimmed();
    }
    // 解析 todirectionJson 字符串
    QJsonDocument jsonDoc = QJsonDocument::fromJson(todirectionJson.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();
    // 遍历每个转辙机元素,并更新其颜色
    for (const QString& switchName : pathElements)
    {
        bool ok;
        switchName.toInt(&ok);
        if (ok)
        {
            QString switchDirection = jsonObj[switchName].toString();
            if (this->m_trackView->m_textMap.contains(switchName))
            {
                SceneText* switchText = this->m_trackView->m_textMap[switchName];
                if (switchDirection == "Normal")
                {
                    switchText->setColor("Green");
                }
                else if (switchDirection == "Reverse")
                {
                    switchText->setColor("Yellow");
                }
                else
                {
                    switchText->setColor("Red");
                }
            }
        }
    }
}
void ViewLogicalControl::drawSwitchsForInroutePath(const QString& routePath, const QString& todirectionJson) {


    // 分割路径字符串并去除首尾空格
    QStringList pathElements = routePath.split(" -> ");
    for (int i = 0; i < pathElements.size(); i++)
    {
        pathElements[i] = pathElements[i].trimmed();
    }
    // 解析 todirectionJson 字符串
    QJsonDocument jsonDoc = QJsonDocument::fromJson(todirectionJson.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();
    // 遍历每个转辙机元素,并绘制线段
    for (int i = 0; i < pathElements.size(); i++)
    {
        QString switchName = pathElements[i];
        bool ok;
        switchName.toInt(&ok);
        if (ok)
        {
            QString switchDirection = jsonObj[switchName].toString();
            QPointF switchPos = switchPositionMap[switchName];
            QPointF nextSwitchPos;
            if (i < pathElements.size() - 1)
            {
                QString nextSwitchName = pathElements[i + 1];
                nextSwitchPos = switchPositionMap[nextSwitchName];
            }
            else
            {
                nextSwitchPos = signalPositionMap.value(pathElements.last());
            }
            QVector2D lineDirection(nextSwitchPos - switchPos);
            lineDirection.normalize();
            QPointF endPos = switchPos + lineDirection.toPointF() * (kSwitchLineLength / 2);
            QPointF startPos = switchPos - lineDirection.toPointF() * (kSwitchLineLength / 2);
            QString lineName = switchName + "->" + (i < pathElements.size() - 1 ? pathElements[i + 1] : pathElements.last());
            // 删除之前从该转辙机出发的所有线段
            removeLinesFromSwitch(switchName);
            QColor lineColor = (switchDirection == "Normal") ? Qt::green : Qt::yellow;
            this->m_trackView->drawPath(lineName, switchPos, endPos, lineColor);
            this->Line2CorlorMap.insert(lineName, lineColor);
        }
    }


}
void ViewLogicalControl::removeLinesFromSwitch(const QString& switchName)
{
    // 遍历 Line2CorlorMap,删除所有以 switchName 开头的线段
    QMap<QString, QColor>::iterator it = this->Line2CorlorMap.begin();
    while (it != this->Line2CorlorMap.end())
    {
        if (it.key().startsWith(switchName + "->"))
        {
            QString lineName = it.key();
            this->m_trackView->removePath(lineName);
            it = this->Line2CorlorMap.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
void ViewLogicalControl::handlerSignalDrawRequestForInroutePath(QString routepath) {
    auto the_last_signal = routepath.split(" -> ").last().trimmed(); //找到最后一个信号机
    auto inwhichline = Configurations::determineSignalType(the_last_signal);
    //不管何时，我们根据方向来寻找所有可能到达的信号机
    QStringList avaibleSignals;  //所有可能遍历到的信号机序列
    QString forwarddirection;
    if (inwhichline == SignalType::DownlineSignal || inwhichline == SignalType::DownwardCXSignal) {
        forwarddirection = da.getCurrentTrackDirectionStatus("Down");
    }
    else {
        forwarddirection = da.getCurrentTrackDirectionStatus("Up");
    }
    if (inwhichline == SignalType::DownlineSignal) {
        if (forwarddirection == "Forward") {
            avaibleSignals = Configurations::downlineTrackTraversing; //我们要怎么遍历
        }
        else {
            QStringList reversedList = Configurations::downlineTrackTraversing;
            std::reverse(reversedList.begin(), reversedList.end());
            avaibleSignals = reversedList;
        }
    }
    if (inwhichline == SignalType::DownwardCXSignal) {
        if (forwarddirection == "Forward") {
            avaibleSignals = Configurations::downlineTrackTraversingCX; //我们要怎么遍历
        }
        else {
            QStringList reversedList = Configurations::downlineTrackTraversingCX;
            std::reverse(reversedList.begin(), reversedList.end());
            avaibleSignals = reversedList;
        }
    }
    if (inwhichline == SignalType::UplineSignal) {
        if (forwarddirection == "Forward") {
            avaibleSignals = Configurations::uplineTraversing; //我们要怎么遍历
        }
        else {
            QStringList reversedList = Configurations::uplineTraversing;
            std::reverse(reversedList.begin(), reversedList.end());
            avaibleSignals = reversedList;
        }
    }
    if (inwhichline == SignalType::UpwardCXSignal) {
        if (forwarddirection == "Forward") {
            avaibleSignals = Configurations::uplineTraversingCX; //我们要怎么遍历
        }
        else {
            QStringList reversedList = Configurations::uplineTraversingCX;
            std::reverse(reversedList.begin(), reversedList.end());
            avaibleSignals = reversedList;
        }
    }
    //还是一样从起始段算起他，找到开始按钮会有什么规律
    //每次都只是修改起始段信号机颜色，以及下行和上行线站外信号机颜色
    //对于站外信号机，我们首先
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
    //auto overlap = std::find_first_of(route.rbegin(), route.rend(), avaibleSignals.begin(), avaibleSignals.end());
    // 如果找到重叠部分
// 如果找到重叠部分
  //  if (overlap != route.rend()) {
        // 检查这个重叠点在线路上的下一个信号机是否也在进路中
  //      auto lineOverlap = std::find(avaibleSignals.begin(), avaibleSignals.end(), *overlap);
   //     if (lineOverlap + 1 != avaibleSignals.end() && std::find(route.begin(), route.end(), *(lineOverlap + 1)) == route.end()) {
            // 如果不在，那么反转线路
    //        std::reverse(avaibleSignals.begin(), avaibleSignals.end());
    //        lineOverlap = std::find(avaibleSignals.begin(), avaibleSignals.end(), *overlap);
    //    }
        // 从线路的这个重叠点开始，向前添加所有的信号机到进路的信号机序列中，直到再次回到这个重叠点
     //   auto nextOverlap = std::find(lineOverlap + 1, avaibleSignals.end(), *overlap);
     //   if (nextOverlap == avaibleSignals.end()) {
     //       nextOverlap = std::find(avaibleSignals.begin(), lineOverlap, *overlap);
    //    }
     //   std::copy(lineOverlap, nextOverlap, std::back_inserter(route));
   // }
    QStringList whichlightstobechanged;//哪些信号机应该被修改颜色
    QStringList whichcolorlightstobechanged;//哪些信号机应该被修改颜色
    // 打印结果
    //我们只是对开始信号机的颜色进行改变
    for (const QString& signalId : route) {
        // Check if the signal is open
        if (this->m_trackView->signalStatusMap.value(signalId) == SignalStatus::Open) {
            // Inner loop: Find the next closed signal and count the number of free sections
            int freeSectionCount = 0;
            int currentIndex = route.indexOf(signalId);
            int nextClosedSignalIndex = -1;
            // Find the next closed signal
            for (int i = currentIndex + 1; i != currentIndex; i = (i + 1) % route.size()) {
                if (this->m_trackView->signalStatusMap.value(route[i]) == SignalStatus::Closed) {
                    nextClosedSignalIndex = i;
                    break;
                }
            }
            // If a closed signal is found, count the number of free sections between the current signal and the closed signal
            if (nextClosedSignalIndex != -1) {
                for (int i = currentIndex; i != nextClosedSignalIndex; i = (i + 1) % route.size()) {
                    QString currentSignalId = route[i];
                    QString nextSignalId = route[(i + 1) % route.size()];
                    // Get the track section between the current signal and the next signal
                    Phy_TrackSection* section = this->friend_device_manager->getTrackSectionBySignalIds(currentSignalId, nextSignalId);
                    if (section && section->getStatus() == "Free") {
                        ++freeSectionCount;
                    }
                }
            }
            // Update the color of the signal based on the number of free sections
            if (freeSectionCount >= 3) {
                //signal->setColorState("Green");
                this->m_trackView->setSignalLightColorInScene(signalId, "Green");
                whichlightstobechanged.append(signalId);
                whichcolorlightstobechanged.append("Green");
            }
            else if (freeSectionCount == 2) {
                this->m_trackView->setSignalLightColorInScene(signalId, "GreenYellow");
                whichlightstobechanged.append(signalId);
                whichcolorlightstobechanged.append("Green");
            }
            else if (freeSectionCount == 1) {
                if (Configurations::instationSignals.contains(signalId) == true) {
                    //这个地方还要判断一下，进路所经过的全部元素的X和Y的值，如果所有进路所经过的元素的X和Y的坐标
                    //
                    QStringList pathelment = routepath.split(" -> ");
                    QStringList signalIdssss;
                    for (const QString& element : pathelment) {
                        QString trimmedElement = element.trimmed();
                        bool isNumber;
                        trimmedElement.toInt(&isNumber);
                        if (!isNumber) {
                            signalIdssss.append(trimmedElement);
                        }
                    }
                    QList<qreal> yValues;
                    for (const QString& signalId : signalIdssss) {
                        if (signalPositionMap.contains(signalId)) {
                            QPointF position = signalPositionMap.value(signalId);
                            yValues.append(position.y());
                        }
                    }
                    bool allEqual = true;
                    if (!yValues.isEmpty()) {
                        qreal firstValue = yValues.first();
                        for (int i = 1; i < yValues.size(); ++i) {
                            if (yValues[i] != firstValue) {
                                allEqual = false;
                                break;
                            }
                        }
                    }
                    if (allEqual == false) {
                        this->m_trackView->setSignalLightColorInScene(signalId, "YellowYellow");
                        whichlightstobechanged.append(signalId);
                        whichcolorlightstobechanged.append("doubleyellow");
                    }
                    else {
                        this->m_trackView->setSignalLightColorInScene(signalId, "Yellow");
                        whichlightstobechanged.append(signalId);
                        whichcolorlightstobechanged.append("Yellow");
                    }
                }
                else {
                    this->m_trackView->setSignalLightColorInScene(signalId, "Yellow");
                    whichlightstobechanged.append(signalId);
                    whichcolorlightstobechanged.append("Yellow");
                }
            }
            else {
                if (Configurations::instationSignals.contains(signalId) == true) {
                    this->m_trackView->setSignalLightColorInScene(signalId, "Yellow");
                    whichlightstobechanged.append(signalId);
                    whichcolorlightstobechanged.append("Yellow");
                }
                else {
                    this->m_trackView->setSignalLightColorInScene(signalId, "Red");
                    whichlightstobechanged.append(signalId);
                    whichcolorlightstobechanged.append("Red");
                }
                //当等于0的时，直接显示为黄灯就
            }
        }
    }
    //需要检查
    emit checklightcolorForInroute(routepath, whichlightstobechanged, whichcolorlightstobechanged); //检验灯光颜色是否匹配
}
//我们来看一下站外信号机的亮灯逻辑
//对于站外信号机，要统计前方完整的空闲区段数量，不管何时在所有站内区段总数的情况下，减去1，
// 完整的是指，进路方向所覆盖的区段要完整覆盖进路中的所有道岔
//当存在道岔反向的时候，还要减一（其实就是紧扣前方空闲完整区段数）。 考虑前方可到达的是指，进路不能越过被处于关闭状态的区段
//我们应该先配置好站内信号机的颜色，然后在对站外信号机颜色做计算。
//也就说，我们要计算的是 进路指定的能到达方向的完整空闲区段数，作为站外信号机的颜色控制。当计算完整空闲区段数之后，还要减去一就对了
//还有一类比较特殊的情况，如果在列车前进方向有引导进路的信号机，即便它是红白颜色，也要跳过，找到第一个呈现出关闭状态的信号机，然后统计前方空闲区段数
//每次在初始化整个应用程序的时候，所有信号机都有出了颜色之外，另外一个属性，叫做是否是开放，是否是关闭，是否是隐藏状态。每次排列了一条进路，就需要设置
//起始段信号机开放就可以了，将进路结束处的信号机隐藏，等到进路解除锁定之后，其实信号机再变成关闭状态。所有站内信号机默认都是关闭状态，所有站内信号
//每当有车占用某个区段的时候，它的保护信号灯颜色都红色，且状态变成关闭状态
//我们再来看下，站内信号机的颜色控制
//不管那种情况了，就看前进方向有多少个区段就行了
//站内信号机还有关闭，开放和隐藏三种状态，首先找到前方第一个红色信号机的，如果在寻找的过程中，发现某个站内信号机是开放的，但是红色的时候，要跳过，继续寻找，然后计算前方空闲的区段个数。
//这里有两种情况需要考虑，一是如果从X开进路到类似于SI之类的话，就是X开放，SI隐藏，但如果从X开进路到XI， X4等，这需要将开始出信号机开放，结束处信号机继续保持关闭状态
void ViewLogicalControl::obtainUpdatedSignalColorMap()
{
    for (const QString& key : this->m_trackView->m_eclipseMap.keys())
    {
        SceneEclipse* eclipse = this->m_trackView->m_eclipseMap[key];
        if (key.contains("-B1") || key.contains("-B2"))
        {
            SceneEclipse* eclipse = this->m_trackView->m_eclipseMap[key];
            signalcolorMap[key] = eclipse->getColorState();
        }
    }
}

//重新从Trackview获取区段颜色，并存储在segmentcolorstatusMap中
void ViewLogicalControl::reloadSectionColorMap() {
    auto myallviewTrackSectionstatus = da.getAllViewTrackSectionStatus();
    for (const QString& key : myallviewTrackSectionstatus.keys())
    {
        auto  the_line_color = myallviewTrackSectionstatus[key];
        sectioncolorstatusMap.insert(key, the_line_color);
    }
}

//重新装载位置
void ViewLogicalControl::reloadSwitchPositionMap() {

        auto tmswpomap = da.getAllTurnoutPositions();
        for (auto it = tmswpomap.begin(); it != tmswpomap.end(); ++it) {
            switchPositionMap.insert(it.key(), it.value());
        }
    
}

void ViewLogicalControl::reloadSignalPositionMap() {

    auto tmswpomap = da.getAllSignalPositions();
    for (auto it = tmswpomap.begin(); it != tmswpomap.end(); ++it) {
        signalPositionMap.insert(it.key(), it.value());
    }

}



QString ViewLogicalControl::findBestMatchedTrack(const QString& routePath)
{
    QStringList pathElements = routePath.split(" -> ");
    QStringList filteredPathElements;
    // 过滤掉进路中的转辙机
    for (const QString& element : pathElements)
    {
        if (!element.contains(QRegularExpression("\\d+")))
        {
            filteredPathElements.append(element);
        }
    }
    // 计算各个线路的覆盖度
    int downlineTrackTraversingCoverage = 0;
    int uplineTraversingCoverage = 0;
    int downlineTrackTraversingCXCoverage = 0;
    int uplineTraversingCXCoverage = 0;
    for (const QString& signal : filteredPathElements)
    {
        if (Configurations::downlineTrackTraversing.contains(signal))
        {
            downlineTrackTraversingCoverage++;
        }
        else if (Configurations::uplineTraversing.contains(signal))
        {
            uplineTraversingCoverage++;
        }
        else if (Configurations::downlineTrackTraversingCX.contains(signal))
        {
            downlineTrackTraversingCXCoverage++;
        }
        else if (Configurations::uplineTraversingCX.contains(signal))
        {
            uplineTraversingCXCoverage++;
        }
    }
    // 找到覆盖度最高的线路
    int maxCoverage = std::max({ downlineTrackTraversingCoverage, uplineTraversingCoverage, downlineTrackTraversingCXCoverage, uplineTraversingCXCoverage });
    if (maxCoverage == downlineTrackTraversingCoverage)
    {
        return "downlineTrackTraversing";
    }
    else if (maxCoverage == uplineTraversingCoverage)
    {
        return "uplineTraversing";
    }
    else if (maxCoverage == downlineTrackTraversingCXCoverage)
    {
        return "downlineTrackTraversingCX";
    }
    else if (maxCoverage == uplineTraversingCXCoverage)
    {
        return "uplineTraversingCX";
    }
    else
    {
        return QString();
    }
}
bool ViewLogicalControl::getSecneSwitchLockStatus(QString switchNumber) {
    return this->switchlockedMap.value(switchNumber, false);
}
//将某个道岔设置为定向
void ViewLogicalControl::handlerSwitchDirectedRequest(QString switchID) {
    //首先删除所有与有关的画图信息
    removeLinesFromSwitch(switchID);
    //第二步设置switch的颜色
    if (switchID == "4") {
        QString lineName = switchID + "->" + "XI";
        QColor lineColor = Qt::green;
        QPointF switchPos = switchPositionMap[switchID];
        QPointF endPos = signalPositionMap.value("XI");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap[switchID];
        switchText->setColor("Green");
        return;
    }
    if (switchID == "2") {
        QString lineName = switchID + "->" + "XII";
        QColor lineColor = Qt::green;
        QPointF switchPos = switchPositionMap[switchID];
        QPointF endPos = signalPositionMap.value("XII");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap[switchID];
        switchText->setColor("Green");
        return;
    }
    if (switchID == "1" || switchID == "7" || switchID == "9") {
        QString lineName = switchID + "->" + "SII";
        QColor lineColor = Qt::green;
        QPointF switchPos = switchPositionMap[switchID];
        QPointF endPos = signalPositionMap.value("SII");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap[switchID];
        switchText->setColor("Green");
        return;
    }
    if (switchID == "5" || switchID == "3" || switchID == "11") {
        QString lineName = switchID + "->" + "SI";
        QColor lineColor = Qt::green;
        QPointF switchPos = switchPositionMap[switchID];
        QPointF endPos = signalPositionMap.value("SI");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap[switchID];
        switchText->setColor("Green");
        return;
    }
    return;
}
void ViewLogicalControl::handlerSwitchReverseRequest(QString switchID) {
    //首先删除所有与有关的画图信息
    //第二步设置switch的颜色

    if (switchID == "4") {
        removeLinesFromSwitch(switchID);
        QString lineName = switchID + "->" + "X3";
        QColor lineColor = Qt::yellow;

        QPointF switchPos = switchPositionMap[switchID];


        QPointF endPos = signalPositionMap.value("X3");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap[switchID];
        switchText->setColor("Yellow");
        return;
    }
    if (switchID == "2") {
        removeLinesFromSwitch(switchID);
        QString lineName = switchID + "->" + "X4";
        QColor lineColor = Qt::yellow;


        QPointF switchPos = switchPositionMap[switchID];


        QPointF endPos = signalPositionMap.value("X4");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap[switchID];
        switchText->setColor("Yellow");
        return;
    }
    if (switchID == "1" || switchID == "3") {
        removeLinesFromSwitch("1");
        removeLinesFromSwitch("3");
        QString lineName = "1->3";
        QColor lineColor = Qt::yellow;

        QPointF switchPos = switchPositionMap["1"];
        QPointF endPos = switchPositionMap.value("3");


        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap["1"];
        switchText->setColor("Yellow");
        lineName = "3->1";

        switchPos = switchPositionMap["3"];
        endPos = switchPositionMap.value("1");


        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText1 = this->m_trackView->m_textMap["3"];
        switchText1->setColor("Yellow");
    }
    if (switchID == "5" || switchID == "7") {
        removeLinesFromSwitch("5");
        removeLinesFromSwitch("7");
        QString lineName = "5->7";
        QColor lineColor = Qt::yellow;

        QPointF switchPos = switchPositionMap["5"];
        QPointF endPos = switchPositionMap.value("7");


        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap["5"];
        switchText->setColor("Yellow");
        lineName = "7->5";


        switchPos = switchPositionMap["7"];
        endPos = switchPositionMap.value("5");



        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText1 = this->m_trackView->m_textMap["7"];
        switchText1->setColor("Yellow");
    }
    if (switchID == "9") {
        removeLinesFromSwitch(switchID);
        QString lineName = switchID + "->" + "S4";
        QColor lineColor = Qt::yellow;


        QPointF switchPos = switchPositionMap[switchID];



        QPointF endPos =signalPositionMap.value("S4");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap[switchID];
        switchText->setColor("Yellow");
        return;
    }
    if (switchID == "11") {
        removeLinesFromSwitch(switchID);
        QString lineName = switchID + "->" + "S3";
        QColor lineColor = Qt::yellow;

        QPointF switchPos = switchPositionMap[switchID];


        QPointF endPos = signalPositionMap.value("S3");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap[switchID];
        switchText->setColor("Yellow");
        return;
    }
    return;
}

//麻痹的，越搞越多任务
void ViewLogicalControl::handlerSwitchLostRequest(QString switchID) {
    if (switchID == "4") {
        removeLinesFromSwitch(switchID);
        QString lineName = switchID + "->" + "X3";
        QColor lineColor = Qt::red;

        QPointF switchPos = switchPositionMap[switchID];
        QPointF endPos = signalPositionMap.value("X3");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap[switchID];
        switchText->setColor("Red");

        lineName = switchID + "->" + "XI";
        switchPos = switchPositionMap[switchID];
        endPos = signalPositionMap.value("XI");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        switchText = this->m_trackView->m_textMap[switchID];
        switchText->setColor("Red");


        return;
    }
    if (switchID == "2") {


        removeLinesFromSwitch(switchID);
        QString lineName = switchID + "->" + "X4";
        QColor lineColor = Qt::red;
        QPointF switchPos = switchPositionMap[switchID];
        QPointF endPos = signalPositionMap.value("X4");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap[switchID];
        switchText->setColor("Red");
        lineName = switchID + "->" + "XII";
        switchPos = switchPositionMap[switchID];
        endPos = signalPositionMap.value("XII");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        switchText = this->m_trackView->m_textMap[switchID];
        switchText->setColor("Red");


        return;
    }


    if (switchID == "1" || switchID == "3") {


        removeLinesFromSwitch("1");
        removeLinesFromSwitch("3");
        //先画1的
        QString lineName = "1->3";
        QColor lineColor = Qt::red;
        QPointF switchPos = switchPositionMap["1"];
        QPointF endPos = switchPositionMap.value("3");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        SceneText* switchText = this->m_trackView->m_textMap["1"];
        switchText->setColor("Red");

        lineName = "1->7";
        switchPos = switchPositionMap["1"];
        endPos = switchPositionMap.value("7");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色

        lineName = "3->1";
        switchPos = switchPositionMap["3"];
        endPos = switchPositionMap.value("1");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        this->m_trackView->m_textMap["3"]->setColor("Red");

        lineName = "3->5";
        switchPos = switchPositionMap["3"];
        endPos = switchPositionMap.value("5");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);


        return;
    }


    if (switchID == "5" || switchID == "7") {


        removeLinesFromSwitch("5");
        removeLinesFromSwitch("7");
        //先画5的
        QString lineName = "5->7";
        QColor lineColor = Qt::red;
        QPointF switchPos = switchPositionMap["5"];
        QPointF endPos = switchPositionMap.value("7");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        this->m_trackView->m_textMap["5"]->setColor("Red");

        lineName = "5->3";
        switchPos = switchPositionMap["5"];
        endPos = switchPositionMap.value("3");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);


        lineName = "7->5";
        switchPos = switchPositionMap["7"];
        endPos = switchPositionMap.value("5");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        this->m_trackView->m_textMap["7"]->setColor("Red");

        lineName = "7->1";
        switchPos = switchPositionMap["7"];
        endPos = switchPositionMap.value("1");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);



        return;
    }


    if (switchID == "9") {


        removeLinesFromSwitch(switchID);
        QString lineName = switchID + "->" + "S4";
        QColor lineColor = Qt::red;
        QPointF switchPos = switchPositionMap[switchID];
        QPointF endPos = signalPositionMap.value("S4");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        this->m_trackView->m_textMap[switchID]->setColor("Red");

        lineName = switchID + "->" + "SII";
        endPos = signalPositionMap.value("SII");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);


        return;
    }
    if (switchID == "11") {


        removeLinesFromSwitch(switchID);
        QString lineName = switchID + "->" + "S3";
        QColor lineColor = Qt::red;
        QPointF switchPos = switchPositionMap[switchID];
        QPointF endPos = signalPositionMap.value("S3");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);
        //与此同时，也要修改switch的text颜色
        this->m_trackView->m_textMap[switchID]->setColor("Red");

        lineName = switchID + "->" + "SI";
        endPos = signalPositionMap.value("SI");
        this->m_trackView->drawPathForSwitch(lineName, switchPos, endPos, lineColor);
        this->Line2CorlorMap.insert(lineName, lineColor);


        return;
    }

}




//当信号机发生变化的时候，需要检查界面上的颜色和沙盘上的信号机颜色是否一致
void ViewLogicalControl::handlerSignalColorUpdatedEvent(QString signalid, QString colorstatus) {
    //如果检查到一致就不做处理，
    //如果检查到冲突逻辑，需要发送到联锁控制器做进一步处理, 发送到联锁控制逻辑进行处理，进行信号灯的联锁控制, 是否都是一致的要在联锁控制中进行发码判断
    obtainUpdatedSignalColorMap();
    auto light_1 = signalcolorMap.value(signalid + "-B1").trimmed();
    auto light_2 = signalcolorMap.value(signalid + "-B2").trimmed();
    auto the_status = light_1 + "|" + light_2;
    if (the_status != colorstatus) {        //发现界面状态和沙盘是不一致的
        emit notifySignalStatusInconsistence(signalid, the_status); //期望达到的颜色是什么
    }
    else {      //通知联锁控制器检查所有信号机和物理信号机状态，敦促其进行发码
        emit notifySignalStatusCheckandPossibleFama(signalcolorMap);
    }
    return;
}

//如果一致的话，则通知联锁控制允许发码，如果不一致的话则通知联锁控制系统做进一步检查
////    "status TEXT CHECK(status IN ('Unknown', 'Free', 'Occupied'))"
void ViewLogicalControl::handlerTrackSectionStatusUpdatedEvent(QString sectionid, QString sectionstatus) {
    //第一步：判断界面上的区段状态是否和下位机的区段状态一致
    //也就说轨道区段只是为了控制灯光颜色，如果与原来的颜色是一致则不进行处理
    qDebug() << "handlerTrackSectionStatusUpdatedEvent......" << sectionid << sectionstatus;
    //这个地方是不对的，应该还是要检查那个数据结构的，麻蛋的

    if (sectioncolorstatusMap.contains(sectionid) == false) {
        sectioncolorstatusMap.insert(sectionid, sectionstatus);
        //发出消息通知view进行重绘区段,并在重新绘制区段的同时，更新数据库中的View_TrackSection表
        //通知联锁控制逻辑，某个区段的状态发生变化
         emit changeTracksectionColorRequest(sectionid, sectionstatus);
         emit notifySectionStatusChange(sectionid, sectionstatus);     //通知联锁控制器
    }




    //for (int i = 0; i < sectioncolorstatusMap.size(); ++i) {
    //    QString secid = sectioncolorstatusMap.k;
    //    QString status = sectioncolorstatusMap[i];

    //    if (physectionmap.contains(id)) {
    //        Phy_TrackSection* section = physectionmap[id];
    //        if (section->getStatus() != status) {
    //            section->setStatus(status);
    //            da.updateTrackSectionStatus(id, status);
    //            emit trackSectionStatusChanged(id, status);
    //        }
    //    }
    //    else {
    //        Phy_TrackSection* newSection = new Phy_TrackSection();
    //        newSection->setTrackSectionId(id);
    //        newSection->setStatus(status);
    //        physectionmap.insert(id, newSection);
    //        emit trackSectionStatusChanged(id, status);
    //    }
    //}



    //reloadSectionColorMap();    //我突然间明白了，还是需要弄这里
    //auto the_Status = this->sectioncolorstatusMap.value(sectionid);
    //if (the_Status == "Red") {  //如果在界面中的是区段红色，而section的，
    //    if (sectionstatus == "Free") {
    //        emit notifySegmentClearEvent(sectionid);
    //    }
    //}

    //if (the_Status == "White") {  //如果在界面中的是红色，而section的
    //    if (sectionstatus == "Occupied") {
    //        emit notifyInrouteSegmentTaken(sectionid);
    //    }
    //}

    //if (the_Status == "Default") {  //如果在界面中的是红色，而section的
    //    if (sectionstatus == "Occupied") {
    //        emit notifySegmentTakenEvent(sectionid);
    //    }
    //}
    return;
}



//当道岔变化的时候，界面上也要发生变化，对于那种有特殊冲突的情况，我们进行特殊处理拉倒
void ViewLogicalControl::handlerTurnoutStatusUpdatedEvent(QString turnoutid, QString status) {
    qDebug() << "我们来处理道岔状态更新消息...." << turnoutid << status;
    if (this->signalPositionMap.size() == 0) {
        reloadSwitchPositionMap();
        reloadSignalPositionMap();
    }
    if (turnoutid == "4" && status == "Reverse") {
        removeLinesFromSwitch(turnoutid);
        lightLogicForSwitchDrawn(turnoutid + "->" + "X3", Qt::yellow);
        return;
    }
    if (turnoutid == "4" && status == "Normal") {
        removeLinesFromSwitch(turnoutid);
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Down");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "SN", Qt::green);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "XI", Qt::green);
        }
        return;
    }
    if (turnoutid == "4" && status == "Lost") {
        removeLinesFromSwitch(turnoutid);
        lightLogicForSwitchDrawn(turnoutid + "->" + "XI", Qt::red);
        lightLogicForSwitchDrawn(turnoutid + "->" + "X3", Qt::red);
        return;
    }
    if (turnoutid == "2" && status == "Reverse") {
        removeLinesFromSwitch(turnoutid);
        lightLogicForSwitchDrawn(turnoutid + "->" + "X4", Qt::yellow);
        return;
    }
    if (turnoutid == "2" && status == "Normal") {
        removeLinesFromSwitch(turnoutid);
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Up");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "XII", Qt::green);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "S", Qt::green);
        }
        return;
    }
    if (turnoutid == "2" && status == "Lost") {
        removeLinesFromSwitch(turnoutid);
        lightLogicForSwitchDrawn(turnoutid + "->" + "XII", Qt::red);
        lightLogicForSwitchDrawn(turnoutid + "->" + "X4", Qt::red);
        return;
    }
    //这面先
    if (turnoutid == "1" && status == "Normal") {
        removeLinesFromSwitch(turnoutid);
        //首先判断方向，方向不同，画出的线的方向也不同
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Up");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "XN", Qt::green);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "7", Qt::green);
        }
        return;
    }
    if (turnoutid == "1" && status == "Reverse") {
        removeLinesFromSwitch(turnoutid);
        lightLogicForSwitchDrawn(turnoutid + "->" + "3", Qt::yellow);
        return;
    }
    if (turnoutid == "1" && status == "Lost") {
        removeLinesFromSwitch(turnoutid);
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Up");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "XN", Qt::red);
            lightLogicForSwitchDrawn(turnoutid + "->" + "3", Qt::red);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "3", Qt::red);
            lightLogicForSwitchDrawn(turnoutid + "->" + "7", Qt::red);
        }
        return;
    }
    if (turnoutid == "7" && status == "Normal") {
        removeLinesFromSwitch(turnoutid);
        //首先判断方向，方向不同，画出的线的方向也不同
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Up");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "1", Qt::green);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "9", Qt::green);
        }
        return;
    }
    if (turnoutid == "7" && status == "Reverse") {
        removeLinesFromSwitch(turnoutid);
        lightLogicForSwitchDrawn(turnoutid + "->" + "5", Qt::yellow);
        return;
    }
    if (turnoutid == "7" && status == "Lost") {
        removeLinesFromSwitch(turnoutid);
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Up");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "5", Qt::red);
            lightLogicForSwitchDrawn(turnoutid + "->" + "1", Qt::red);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "9", Qt::red);
            lightLogicForSwitchDrawn(turnoutid + "->" + "5", Qt::red);
        }
        return;
    }
    if (turnoutid == "9" && status == "Normal") {
        removeLinesFromSwitch(turnoutid);
        //首先判断方向，方向不同，画出的线的方向也不同
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Up");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "7", Qt::green);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "SII", Qt::green);
        }
        return;
    }
    if (turnoutid == "9" && status == "Reverse") {
        removeLinesFromSwitch(turnoutid);
        lightLogicForSwitchDrawn(turnoutid + "->" + "S4", Qt::yellow);
        return;
    }
    if (turnoutid == "9" && status == "Lost") {
        removeLinesFromSwitch(turnoutid);
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Up");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "7", Qt::red);
            lightLogicForSwitchDrawn(turnoutid + "->" + "S4", Qt::red);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "S4", Qt::red);
            lightLogicForSwitchDrawn(turnoutid + "->" + "SII", Qt::red);
        }
        return;
    }
    //.....................
    if (turnoutid == "5" && status == "Normal") {
        removeLinesFromSwitch(turnoutid);
        //首先判断方向，方向不同，画出的线的方向也不同
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Down");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "3", Qt::green);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "X", Qt::green);
        }
        return;
    }
    if (turnoutid == "5" && status == "Reverse") {
        removeLinesFromSwitch(turnoutid);
        lightLogicForSwitchDrawn(turnoutid + "->" + "7", Qt::yellow);
        return;
    }
    if (turnoutid == "5" && status == "Lost") {
        removeLinesFromSwitch(turnoutid);
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Down");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "7", Qt::red);
            lightLogicForSwitchDrawn(turnoutid + "->" + "3", Qt::red);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "X", Qt::red);
            lightLogicForSwitchDrawn(turnoutid + "->" + "7", Qt::red);
        }
        return;
    }
    if (turnoutid == "3" && status == "Normal") {
        removeLinesFromSwitch(turnoutid);
        //首先判断方向，方向不同，画出的线的方向也不同
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Down");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "11", Qt::green);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "5", Qt::green);
        }
        return;
    }
    if (turnoutid == "3" && status == "Reverse") {
        removeLinesFromSwitch(turnoutid);
        lightLogicForSwitchDrawn(turnoutid + "->" + "1", Qt::yellow);
        return;
    }
    if (turnoutid == "3" && status == "Lost") {
        removeLinesFromSwitch(turnoutid);
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Down");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "11", Qt::red);
            lightLogicForSwitchDrawn(turnoutid + "->" + "1", Qt::red);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "5", Qt::red);
            lightLogicForSwitchDrawn(turnoutid + "->" + "1", Qt::red);
        }
        return;
    }
    if (turnoutid == "11" && status == "Normal") {
        removeLinesFromSwitch(turnoutid);
        //首先判断方向，方向不同，画出的线的方向也不同
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Down");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "SI", Qt::green);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "3", Qt::green);
        }
        return;
    }
    if (turnoutid == "11" && status == "Reverse") {
        removeLinesFromSwitch(turnoutid);
        lightLogicForSwitchDrawn(turnoutid + "->" + "S3", Qt::yellow);
        return;
    }
    if (turnoutid == "11" && status == "Lost") {
        removeLinesFromSwitch(turnoutid);
        auto t_trackstatus = da.getCurrentTrackDirectionStatus("Down");
        if (t_trackstatus == "Forward") {
            lightLogicForSwitchDrawn(turnoutid + "->" + "SI", Qt::red);
            lightLogicForSwitchDrawn(turnoutid + "->" + "S3", Qt::red);
        }
        else {
            lightLogicForSwitchDrawn(turnoutid + "->" + "S3", Qt::red);
            lightLogicForSwitchDrawn(turnoutid + "->" + "3", Qt::red);
        }
        return;
    }
    return;
}


//控制场景视图中的区段颜色变化
void ViewLogicalControl::controlTrackSectionColor(QString sectionid, QString sectionstatus) {
    //所有访问的地方都必须修改
    //在更新了界面中的颜色的时候，必须更新对应数据库中的颜色信息
    emit changeTracksectionColorRequest(sectionid, sectionstatus);

    return;
}



void ViewLogicalControl::lightLogicForSwitchDrawn(QString lineName, QColor linecolor) {
    QStringList parts = lineName.split("->");
    QString startName = parts.first().trimmed();
    QString endName = parts.last().trimmed();
    QPointF switchPos = switchPositionMap[startName];
    QPointF endPos;
    if (Utilty::isAllDigits(endName)) {
        endPos = switchPositionMap[endName];
    }
    else {
        endPos = signalPositionMap[endName];
    }
    qDebug() << "the postion location is ....HHHH" << switchPos << endPos;
    emit drawPathForTurnoutChange(lineName, switchPos, endPos, linecolor);
    this->Line2CorlorMap.insert(lineName, linecolor);
    QString colorname;
    if (linecolor == Qt::red) {
        colorname = "Red";
    }
    else if (linecolor == Qt::yellow) {
        colorname = "Yellow";
    }
    else {
        colorname = "Green";
    }
    emit drawTurnoutTextForTurnoutChange(startName, colorname);
}


