#include "ToplogyScene.h"
#include <QApplication>
#include <QEvent>
#include "Interlockingevent.h"
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include "Buttonwidget.h"
#include <QTime>
#include <QTimer>
#include <QInputDialog>
#include <QDir>
#include "Helper.h"
#include <QQueue>
#include <QVector>
#include <algorithm>
#include <QtCore/qmath.h>
#include <QDebug>

moodycamel::ConcurrentQueue<std::string> commanditems;  //多个线程执行
typedef struct
{
    unsigned char id;
    QByteArray ba;
}CommmandClass;

bool compare(CommmandClass command1, CommmandClass command2) {
    return command1.id < command2.id;
}

std::string ToplogyScene::appendCRC(QByteArray ba){
    QByteArray bb;
    bb.resize(ba.size()+2);
    uint16_t wcrc1 = Helper::ModbusCRC16(ba);
    unsigned char di1=uint8_t(wcrc1);
    unsigned char gao1=uint8_t(wcrc1>>8);
    int i=0;
    for(i=0;i<ba.size();i++){
        bb[i]=ba[i];
    }
    bb[i++]=di1;
    bb[i++]=gao1;
    return bb.toStdString();
}

ToplogyScene::ToplogyScene(QObject *parent) :
    QGraphicsScene(parent)
{
    m_bPressed=false; //初始化时，鼠标没有被按下
    startupfaflag=0;
    sendingchar = 0xc0;
    startTimer(1000);                 //每隔100秒中检查状态信息，在这个里面完成发码控制,最小值300
    connect(this,&ToplogyScene::allowRouteSignal,this,&ToplogyScene::TryRouterPermitAndDrwaScene);
    signallist = new CLinkList();
    signallist->initCLinkList();
    signallist->insertLNode(1,"X");
    signallist->insertLNode(2,"XI");
    signallist->insertLNode(3,"SN");
    signallist->insertLNode(4,"1101");
    signallist->insertLNode(5,"1103");
    signallist->insertLNode(6,"1105");
    signallist->insertLNode(7,"1107");
    segmentlist  = new CLinkList();
    segmentlist->initCLinkList();
    segmentlist->insertLNode(1,"3-11G");
    segmentlist->insertLNode(2,"1G");
    segmentlist->insertLNode(3,"4DG");
    segmentlist->insertLNode(4,"X1LQG");
    segmentlist->insertLNode(5,"1101G");
    segmentlist->insertLNode(6,"1103G");
    segmentlist->insertLNode(7,"1105G");
    segmentlist->insertLNode(8,"1107G");
    //建立区段与信号灯防护关系,例如信号灯,X防护了两个区段
    signal_prote_segement["X"]="3-11G|1G";
    signal_prote_segement["XI"]="4DG";
    signal_prote_segement["SN"]="X1LQG";
    signal_prote_segement["1101"]="1101G";
    signal_prote_segement["1103"]="1103G";
    signal_prote_segement["1105"]="1105G";
    signal_prote_segement["1107"]="1107G";
    qudongzhilingmap.insert(0x90,"-|-|-|-|-|-|-|-|-|-|-|4#SFJ|4#FCJ|4#DCJ|4#FC|4#DC");
    qudongzhilingmap.insert(0x91,"-|1101H|1101U|1101L|-|X1H|X1U|X1L|1101LUJ|1101UJ|1101HJ|1101LJ|X1LUJ|X1LJ|X1LXJ|FJ");
    qudongzhilingmap.insert(0x92,"-|-|2#FC|2#DC|11#FC|11#DC|9#FC|9#DC|7#FC|7#DC|5#FC|5#DC|3#FC|3#DC|1#FC|1#DC");
    qudongzhilingmap.insert(0x93,"-|-|S1-FB|S1-U|S1-H|S1-L|XN-YB|XN-2U|XN-H|XN-L|XN-U|X-YB|X-2U|X-H|X-L|X-U");
    qudongzhilingmap.insert(0x94,"-|-|XII-H|XII-L|S4-FB|S4-U|S4-H|S4-L|S3-FB|S3-U|S3-H|S3-L|SII-FB|SII-U|SII-H|SII-L");
    qudongzhilingmap.insert(0x95,"-|X4-H|X4-L|X3-U|X3-H|X3-L|SN-YB|SN-2U|SN-H|SN-L|SN-U|S-YB|S-2U|S-H|S-L|S-U");
    qudongzhilingmap.insert(0x96,"-|-|-|-|-|1107-U|1107-H|1107-L|-|-|1105-U|1105-H|1105-L|1103-U|1103-H|1103-L");
    qudongzhilingmap.insert(0x97,"-|-|1108-U|1108-H|1108-L|1106-U|1106-H|1106-L|-|-|1104-U|1104-H|1104-L|1102-U|1102-H|1102-L");
    bit2colorcolor.insert(0, 0x7f);
    bit2colorcolor.insert(1, 0xbf);
    bit2colorcolor.insert(2, 0xdf);
    bit2colorcolor.insert(3, 0xef);
    bit2colorcolor.insert(4, 0xf7);
    bit2colorcolor.insert(5, 0xfb);
    bit2colorcolor.insert(6, 0xfd);
    bit2colorcolor.insert(7, 0xfe);
    bit2colorcolor.insert(8, 0x7f);
    bit2colorcolor.insert(9, 0xbf);
    bit2colorcolor.insert(10, 0xdf);
    bit2colorcolor.insert(11, 0xef);
    bit2colorcolor.insert(12, 0xf7);
    bit2colorcolor.insert(13, 0xfb);
    bit2colorcolor.insert(14, 0xfd);
    bit2colorcolor.insert(15, 0xfe);
}

void ToplogyScene::initialfamacomand(){
    interlockinglogic_step2();
}

//包括第4#道岔
void ToplogyScene::guidaoSwitchIntialStatus(){
    QByteArray temp_data1;  //1-3-5-7-9-11-2 道岔定操
    temp_data1.resize(3);
    temp_data1[0] = 0x92;
    temp_data1[1] = 0xea;
    temp_data1[2] = 0xaa;
    this->pushCommanItemToQueue(QByteArray::fromStdString(appendCRC(temp_data1)));

    //4号道岔定操
    QByteArray tempddd = driver4daocha("DC");
    this->pushCommanItemToQueue(tempddd);

}


void ToplogyScene::guidaoSignalInitalStatus(){
    QByteArray temp_data1;
    temp_data1.resize(3);
    temp_data1[0] = 0x97;
    temp_data1[1] = 0xff & Helper::color2byte_1108_L & Helper::color2byte_1106_L;
    temp_data1[2] = 0xff & Helper::color2byte_1104_L & Helper::color2byte_1104_U & Helper::color2byte_1102_U;
    this->pushCommanItemToQueue(QByteArray::fromStdString(appendCRC(temp_data1)));
    temp_data1.resize(3);
    temp_data1[0] = 0x94;
    temp_data1[1] = 0xff & Helper::color2byte_S4_H & Helper:: color2byte_XII_H;
    temp_data1[2] = 0xff & Helper::color2byte_SII_H & Helper:: color2byte_S3_H;
    this->pushCommanItemToQueue(QByteArray::fromStdString(appendCRC(temp_data1)));
    temp_data1.resize(3);
    temp_data1[0] = 0x93;
    temp_data1[1] = 0xff & Helper::color2byte_S1_H;
    temp_data1[2] = 0xff & Helper::color2byte_XN_H & Helper::color2byte_X_H;
    qDebug()<<"在初始化阶段，发送信号灯XN,S1,X操作指令.........";
    this->pushCommanItemToQueue(QByteArray::fromStdString(appendCRC(temp_data1)));
    //1103，1105，1107控制命令,经过检验，是对的
    temp_data1[0] = 0x96;
    temp_data1[1] = 0xff & Helper::color2byte_1107_U;
    temp_data1[2] = 0xff & Helper::color2byte_1105_L & Helper::color2byte_1105_U & Helper::color2byte_1103_L;
    qDebug()<<"在初始化阶段，发送信号灯1103，1105，1107操作指令.........";
    this->pushCommanItemToQueue(QByteArray::fromStdString(appendCRC(temp_data1)));
    temp_data1[0] = 0x91;           //驱动非实物转辙机定操
    temp_data1[1] = 0xff;
    temp_data1[2] = 0xff & Helper::color2byte_XI_H & Helper::color2byte_1101_L;
    qDebug()<<"在初始化阶段，发送实物信号机1101，XI操作指令.........";
    this->pushCommanItemToQueue(QByteArray::fromStdString(appendCRC(temp_data1)));
    temp_data1[0] = 0x95;
    temp_data1[1] =  0xff & Helper::color2byte_X4_H & Helper::color2byte_X3_H;
    temp_data1[2] = 0xff & Helper::color2byte_SN_H & Helper::color2byte_S_H;
    qDebug()<<"在初始化阶段，发送信号灯S,SN,X3,X4操作指令.........";
    this->pushCommanItemToQueue(QByteArray::fromStdString(appendCRC(temp_data1)));
}

QString ToplogyScene::getProtectedSignalAheadForSegment(QString segment_name){
    int pos=0;
    auto tempbool= segmentlist->locateLNode(pos,segment_name);
    if(tempbool==true){
        QVector<int> temparr;        //区段对应的在循环列表中的序号
        for(int i=1;i<=segmentlist->countNode-1;i++){
            auto zz= pos+i;
            if(zz>segmentlist->countNode){
                temparr.append(zz%(segmentlist->countNode));
            }else{
                temparr.append(zz);
            }
        }
        QList<QString> signalss; //所有保护这些区段的信号灯
        //获取在前面有多少个可用分段
        for(int i=0;i<temparr.size();i++){
            auto tmpssssss = segmentlist->getNodeDataByPosition(temparr.at(i));
            auto tmpzz =getSignalBySegmentprotected(tmpssssss);
            signalss.append(tmpzz);
        }
        signalss.removeDuplicates();
        //auto temp_head_signal =signalss.first();
        return signalss.first();
    }
    return "";
}

QString ToplogyScene::getmaxuwhilesegmentistaken(QString seg){
    QString fama="";
    QMap<QString,QString> segment2fama;
    QString current_segment_fama="HU";
    QString current_train_name="harmony";               //假设当前列车名称是和谐号，之所以保留这个是因为后续会增加其它车辆追踪行始的情况
    auto preservered_route = this->anchortracktoplogy->reseveredPathForTheTrain.value(current_train_name,"");
    //找到1107轨道的前一个区段，然后判断是否有安排进路
    auto signalhead = getProtectedSignalAheadForSegment(seg);
    if(preservered_route==""){  //如果没有为车安排任何进路
        auto a1 = (this->signal2Color.contains(signalhead+"-"+"B1")==true) ?this->signal2Color[signalhead+"-"+"B1"]: "black";
        auto a2 = (this->signal2Color.contains(signalhead+"-"+"B2")==true) ?this->signal2Color[signalhead+"-"+"B2"]: "black";
        if(a1=="red" || a2=="red"){
            current_segment_fama ="HU";
        }else{
            current_segment_fama ="JC";
        }
    }
    else
    {
        if(preservered_route.split(">")[0]==signalhead){  //如果安排了进路，且进路信号灯颜色不为红灯时候，当前所占用区段发JC码，否则发红黄码
            auto a1 = (this->signal2Color.contains(signalhead+"-"+"B1")==true) ?this->signal2Color[signalhead+"-"+"B1"]: "black";
            auto a2 = (this->signal2Color.contains(signalhead+"-"+"B2")==true) ?this->signal2Color[signalhead+"-"+"B2"]: "black";
            if(a1=="red" || a2=="red"){
                current_segment_fama ="HU";
            }else{
                current_segment_fama ="JC";
            }
        }else{
            auto a1 = (this->signal2Color.contains(signalhead+"-"+"B1")==true) ?this->signal2Color[signalhead+"-"+"B1"]: "black";
            auto a2 = (this->signal2Color.contains(signalhead+"-"+"B2")==true) ?this->signal2Color[signalhead+"-"+"B2"]: "black";
            if(a1=="red" || a2=="red"){
                current_segment_fama ="HU";
            }else{
                current_segment_fama ="JC";
            }
        }
    }
    segment2fama[seg] =current_segment_fama;
    for(int i=8;i>=1;i--){
        auto tempseg = segmentlist->getNodeDataByPosition(i);
        if(tempseg==seg){
            continue;
        }else{
            segment2fama[tempseg] =generateFaMaSerial(tempseg);
        }
    }
    //根据segment2fama得到所有发码序列,这个地方需要变化一下，然后切换
    QString fama11="";
    QString segment_name="X1LQG";
    fama11.prepend(segment2fama.value(segment_name));
    fama11.prepend("|");
    segment_name="1101G";
    fama11.prepend(segment2fama.value(segment_name));
    fama11.prepend("|");
    segment_name="1103G";
    fama11.prepend(segment2fama.value(segment_name));
    fama11.prepend("|");
    segment_name="1105G";
    fama11.prepend(segment2fama.value(segment_name));
    fama11.prepend("|");
    segment_name="1107G";
    fama11.prepend(segment2fama.value(segment_name));
    fama11.prepend("|");
    segment_name="3-11G";
    fama11.prepend(segment2fama.value(segment_name));
    fama11.prepend("|");
    segment_name="1G";
    fama11.prepend(segment2fama.value(segment_name));
    fama11.prepend("|");
    segment_name="4DG";
    fama11.prepend(segment2fama.value(segment_name));
    fama11.prepend("|");
    fama11=fama11.mid(1);
    return fama11;
}

//QString switchs = "1|3|5|7|9|11|2|4";

void  ToplogyScene::checkSwitchsDirections(QString switchs){
    auto switchlist =switchs.split("|");
    foreach(QString item, switchlist){
        auto status=this->anchortracktoplogy->getSwitchStatus(item);
        if(status==0){  //定向
            this->switch2Direction[item]="directed";
            emit swithoperationsignaldisplay(item, "directed");
        }else if(status==1){    // segment is taken by the train, we can changed signal color in this place
          this->switch2Direction[item]="reversed";
          emit swithoperationsignaldisplay(item, "reversed");
        }else{
            continue;
        }
    }
    return;
}

//  QString segments = "1101G|X1LQG|4DG|1G|3-11G";
//  QString segments = "1107G|1105G|1103G";
//The function update the scene and the segment2Stauts for building track fama
void ToplogyScene::checkSegmentTakenStatus(QString segments){

    auto segementlist =segments.split("|"); //check the status and draw red line if the segement is taken by train
    qDebug()<<"进入到轨道区段状态检测和绘图代码部分checkSegmentTakenStatus...."<<segments;
    foreach(QString item, segementlist){
        auto status=this->anchortracktoplogy->getSegmentStatus(item);
        if(status==0){  //not taken
            //if not taken,draw origin color in the segment
            this->segment2Stauts[item]="NotTaken";

           //第一情况就是没有任何进路排列，车放下去又拿起来
            //这个全是状态空间
            if(this->anchortracktoplogy->reseveredPath.size()==0){  //没有任何进路，而且没有任何


            }



            if(item=="1101G"){
                emit drawlineSignal("1101","1103","origin");
            }else if(item=="X1LQG"){
                emit drawlineSignal("SN","1101","origin");
            }
            else if(item=="1G"){
                emit drawlineSignal("SI","XI","origin");
            }
            else if(item=="4DG"){
                emit drawlineSignal("XI","SN","origin");
            }else if(item=="3-11G"){
                emit drawlineSignal("SI","X","origin");
            }else if(item=="1107G"){
                emit drawlineSignal("1107","1107_right","origin");
                emit drawlineSignal("1107_left","X","origin");
                //当轨道电路上没有车时，其界面上的防护灯的颜色需要变化，沙盘上的也需要变化 每一个转辙机状态所对应的界面也应该画出来
                //根据联锁关系来确定变化
            }else if(item=="1105G"){
                emit drawlineSignal("1105","1107","origin");
            }else if(item=="1103G"){

                emit drawlineSignal("1103","1105","origin");
            }
            else{
                continue;
            }
        }else if(status==1){    // segment is taken by the train, we can changed signal color in this place

            this->segment2Stauts[item]="Taken";
            if(item=="1101G"){
                emit drawlineSignal("1101","1103","red");
                emit signalShouldDisplayStatus("1101-B1","red");
                emit signalShouldDisplayStatus("1101-B2","black");
                auto ba=driver1101_X1command("1101","H");
                this->pushCommanItemToQueue(ba);

            }else if(item=="X1LQG"){
                emit drawlineSignal("SN","1101","red");
                emit signalShouldDisplayStatus("SN-B1","red");
                emit signalShouldDisplayStatus("SN-B2","black");
                auto ba = driverSNColoarComand("H");
                this->pushCommanItemToQueue(ba);
            }
            else if(item=="4DG"){
                emit drawlineSignal("XI","SN","red");
                emit signalShouldDisplayStatus("XI-B1","red");
                emit signalShouldDisplayStatus("XI-B2","black");
                auto ba=driver1101_X1command("X1","H");
                this->pushCommanItemToQueue(ba);
            }else if(item=="3-11G"){
                emit drawlineSignal("SI","X","red");
                emit signalShouldDisplayStatus("X-B1","red");
                emit signalShouldDisplayStatus("X-B2","black");
                auto ba = driverXColorCommand("H");
                this->pushCommanItemToQueue(ba);
            }
            else if(item=="1G"){
                emit drawlineSignal("SI","XI","red");
                emit signalShouldDisplayStatus("X-B1","red");
                emit signalShouldDisplayStatus("X-B2","black");
                auto ba = driverXColorCommand("H");
                this->pushCommanItemToQueue(ba);
            }
            else if(item=="1107G"){//我们还需要记住转辙机当前状态
                emit drawlineSignal("1107","1107_right","red");
                emit drawlineSignal("1107_left","X","red");
                emit signalShouldDisplayStatus("1107-B1","red");
                emit signalShouldDisplayStatus("1107-B2","black");
                auto ba=driver1107ColoarComand("H");
                this->pushCommanItemToQueue(ba);

            }else if(item=="1105G")  //但是1103G和1105G没有被占用啊，这个地方要看下撒子情况
            {
                emit drawlineSignal("1105","1107","red");
                emit signalShouldDisplayStatus("1105-B1","red");
                emit signalShouldDisplayStatus("1105-B2","black");
                auto ba=driver1105ColoarComand("H");
                this->pushCommanItemToQueue(ba);

            }else if(item=="1103G")
            {
                emit drawlineSignal("1103","1105","red");
                emit signalShouldDisplayStatus("1103-B1","red");
                emit signalShouldDisplayStatus("1103-B2","black");
                auto ba=driver1103ColoarComand("H");
                this->pushCommanItemToQueue(ba);
            }
            else{
                continue;
            }
        }else{
            continue;
        }
    }
}

void ToplogyScene::interlockinglogic_step3(QString maxu,  QVector<QString> alreadychangedSigals){
    QString segment_name="X1LQG|3-11G|1101G|1G|1103G|4DG|1105G|1107G";
    auto segments = segment_name.split("|");
    auto mxs =maxu.split("|");
    for(int i=0;i<segments.size();i++){
        auto seg =segments.at(i);  // target seg
        auto getprotectedsiganl =getSignalBySegmentprotected(seg);
        if(alreadychangedSigals.contains(getprotectedsiganl)==true){
            continue;
        }else{
            auto zzmaxu = mxs.at(i);     // target quduan fama
            if(zzmaxu=="U"){
                if(getprotectedsiganl=="1107" || getprotectedsiganl=="1105" || getprotectedsiganl=="1103" ){
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B1","yellow");
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B2","black");
                    emit emitDriverCommand(this->driver110311051107command(getprotectedsiganl,"U"));
                }else if(getprotectedsiganl=="1101"){
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B1","yellow");
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B2","black");
                    emit emitDriverCommand(this->driver1101_X1command(getprotectedsiganl,"U"));
                }else {
                    continue; // others not considered, because there are the in-station signals
                }
            }else if(zzmaxu=="LU"){

                if(getprotectedsiganl=="1107" || getprotectedsiganl=="1105" || getprotectedsiganl=="1103" ){
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B1","yellow");
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B2","green");
                    emit emitDriverCommand(this->driver110311051107command(getprotectedsiganl,"U"));
                }else if(getprotectedsiganl=="1101"){
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B1","yellow");
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B2","green");
                    emit emitDriverCommand(this->driver1101_X1command(getprotectedsiganl,"LU"));
                }else {
                    continue; // others not considered, because there are the in-station signals
                }

            }else if(zzmaxu=="L"){
                if(getprotectedsiganl=="1107" || getprotectedsiganl=="1105" || getprotectedsiganl=="1103" ){
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B1","green");
                    emit emitDriverCommand(this->driver110311051107command(getprotectedsiganl,"L"));
                }else if(getprotectedsiganl=="1101"){
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B1","green");
                    emit emitDriverCommand(this->driver1101_X1command(getprotectedsiganl,"L"));
                }else {
                    continue;
                }

            }else if (zzmaxu=="L2"){

                if(getprotectedsiganl=="1107" || getprotectedsiganl=="1105" || getprotectedsiganl=="1103" ){
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B1","green");
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B2","green");
                    emit emitDriverCommand(this->driver110311051107command(getprotectedsiganl,"L"));
                }else if(getprotectedsiganl=="1101"){
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B1","green");
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B2","green");
                    emit emitDriverCommand(this->driver1101_X1command(getprotectedsiganl,"L"));
                }else {
                    continue;
                }

            }else if (zzmaxu=="L3"){
                if(getprotectedsiganl=="1107" || getprotectedsiganl=="1105" || getprotectedsiganl=="1103" ){
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B1","green");
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B2","green");
                    emit emitDriverCommand(this->driver110311051107command(getprotectedsiganl,"L"));
                }else if(getprotectedsiganl=="1101"){
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B1","green");
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B2","green");
                    emit emitDriverCommand(this->driver1101_X1command(getprotectedsiganl,"L"));
                }else {
                    continue;
                }
            }
            else if (zzmaxu=="HU"){

                if(getprotectedsiganl=="1107" || getprotectedsiganl=="1105" || getprotectedsiganl=="1103" ){
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B1","yellow");
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B2","black");
                    emit emitDriverCommand(this->driver110311051107command(getprotectedsiganl,"U"));
                }else if(getprotectedsiganl=="1101"){
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B1","yellow");
                    emit signalShouldDisplayStatus(getprotectedsiganl+"-B2","black");
                    emit emitDriverCommand(this->driver1101_X1command(getprotectedsiganl,"U"));
                }else {
                    continue;
                }
            }else{
                continue;
            }
        }
    }
}

QString ToplogyScene::interlockinglogic_step2(){

    QString fama ="";
    QString segment_name="X1LQG";
    auto zz=generateFaMaSerial(segment_name);
    fama.append(zz);
    fama.append("|");
    segment_name="3-11G";
    zz=generateFaMaSerial(segment_name);
    fama.append(zz);
    fama.append("|");
    segment_name="1101G";
    zz=generateFaMaSerial(segment_name);
    fama.append(zz);
    fama.append("|");
    segment_name="1G";
    zz=generateFaMaSerial(segment_name);
    fama.append(zz);
    fama.append("|");
    segment_name="1103G";
    zz=generateFaMaSerial(segment_name);
    fama.append(zz);
    fama.append("|");
    segment_name="4DG";
    zz=generateFaMaSerial(segment_name);
    fama.append(zz);
    fama.append("|");
    segment_name="1105G";
    zz=generateFaMaSerial(segment_name);
    fama.append(zz);
    fama.append("|");
    segment_name="1107G";
    zz=generateFaMaSerial(segment_name);
    fama.append(zz);
    emit emitmaxubiao(fama);
    return fama;
}

QVector<QString> ToplogyScene::interlockinglogic_step1(){

    QVector<QString> alreadyChangedSignals;
    alreadyChangedSignals.clear();
    if(this->segment2Stauts.contains("1107G")==true){
        if(this->segment2Stauts["1107G"]=="Taken"){
            emit signalShouldDisplayStatus("1107-B1","red");
            emit signalShouldDisplayStatus("1107-B2","black");
            emit emitDriverCommand(this->driver110311051107command("1107","H"));
            alreadyChangedSignals.append("1107");
        }
    }
    if(this->segment2Stauts.contains("1105G")==true){
        if(this->segment2Stauts["1105G"]=="Taken"){
            emit signalShouldDisplayStatus("1105-B1","red");
            emit signalShouldDisplayStatus("1105-B2","black");
            emit emitDriverCommand(this->driver110311051107command("1105","H"));
            alreadyChangedSignals.append("1105");
        }
    }
    if(this->segment2Stauts.contains("1103G")==true){
        if(this->segment2Stauts["1103G"]=="Taken"){
            emit signalShouldDisplayStatus("1103-B1","red");
            emit signalShouldDisplayStatus("1103-B2","black");
            emit emitDriverCommand(this->driver110311051107command("1103","H"));
            alreadyChangedSignals.append("1103");
        }
    }

    if(this->segment2Stauts.contains("1101G")==true){
        if(this->segment2Stauts["1101G"]=="Taken"){
            emit signalShouldDisplayStatus("1101-B1","red");
            emit signalShouldDisplayStatus("1101-B2","black");
            emit emitDriverCommand(this->driver1101_X1command("1101","H"));
            alreadyChangedSignals.append("1101");
        }
    }

    if(this->segment2Stauts.contains("X1LQG")==true){
        if(this->segment2Stauts["X1LQG"]=="Taken"){
            emit signalShouldDisplayStatus("SN-B1","red");
            emit emitDriverCommand(this->driverSSNX3X4command("SN","H"));
            alreadyChangedSignals.append("SN");
        }
    }
    return alreadyChangedSignals;
}

//每次轨道电路发生变化了，就开始改变场景和轨道上的信号灯颜色
//based on all collected information such as train segement status, preserved routers and its directions
//the functions will change the color of the toplogy and update corresponding scnece color
//  QString segments = "1101G|X1LQG|4DG|1G|3-11G";
//  QString segments = "1107G|1105G|1103G";
void ToplogyScene::interlockingLogicProcess(){

    if(this->anchortracktoplogy->reseveredPath.size()==0){  //there were no route requested
        //auto alreadsignals = interlockinglogic_step1();
       // auto maxu =interlockinglogic_step2();
        //interlockinglogic_step3(maxu,alreadsignals);
    }else{
        // when preserved routes exists, we shoud tackle all signals
        // every time when the track circle changes, we should observed the current segment
          for(int segid=1;segid<=this->segmentlist->countNode;segid++){
               auto sgment_name = segmentlist->getNodeDataByPosition(segid);
               //we continue use the aforementhiond method to change color
               if(this->segment2Stauts[sgment_name]=="Taken"){
                 auto whichsiganl =getSignalBySegmentprotected(sgment_name);
                 if(whichsiganl=="X"){
                     emit signalShouldDisplayStatus("X-B1","red");
                     emit signalShouldDisplayStatus("X-B2","black");
                     emit emitDriverCommand(driverXXNS1command("X","H"));
                 }
                 if(whichsiganl=="XI" || whichsiganl=="1101"){
                     emit signalShouldDisplayStatus(whichsiganl+"-B1","red");
                     emit signalShouldDisplayStatus(whichsiganl+"-B2","black");
                     emit emitDriverCommand(driver1101_X1command(whichsiganl,"H"));
                 }
                 if(whichsiganl=="SN"){
                     emit signalShouldDisplayStatus("SN-B1","red");
                     emit signalShouldDisplayStatus("SN-B2","black");
                     emit emitDriverCommand(driverSSNX3X4command("SN","H"));
                 }

                 if(whichsiganl=="1103" || whichsiganl=="1105" || whichsiganl=="1107" ){
                     emit signalShouldDisplayStatus(whichsiganl+"-B1","red");
                     emit signalShouldDisplayStatus(whichsiganl+"-B2","black");
                     emit emitDriverCommand(driver110311051107command(whichsiganl,"H"));
                 }
               }
          }
          auto maxu =interlockinglogic_step2();
    }
}


//whichoperation：{LU,U,H,L}, 这个是非常特殊的
QByteArray ToplogyScene::driver1101_X1command(QString whichsignal,QString whichoperation){

    QString currentStatusof1101;
    QString currentStatusofX1;

    QByteArray tempba;
    tempba.resize(3);
    tempba[0]=0x91;
    tempba[1]=0xff;  //驱动的时候1101在前，X1在后 组成一个字节
    QMap<QString,unsigned char> driveX1colarbytes;
    driveX1colarbytes["H"]=0xff;
    driveX1colarbytes["U"]=0xfd;
    driveX1colarbytes["LU"]=0xf5;
    driveX1colarbytes["L"]=0xf9;

    QMap<QString,unsigned char> drive1101colarbytes;
    drive1101colarbytes["H"]=0xdf;
    drive1101colarbytes["U"]=0xbf;
    drive1101colarbytes["LU"]=0x7f;
    drive1101colarbytes["L"]=0xef;


    if(this->anchortracktoplogy->signalsStatus.contains("1101|DJ")==false){
        return NULL;
    }
    if(whichsignal=="X1"){

        //获取1101状态
        if(this->anchortracktoplogy->signalsStatus["1101|DJ"]==0x00
                && this->anchortracktoplogy->signalsStatus["1101|HJ"]==0x00  ) {
                currentStatusof1101="H";
        }
        if(this->anchortracktoplogy->signalsStatus["1101|2DJ"]==0x00
                &&  this->anchortracktoplogy->signalsStatus["1101|DJ"]==0x01
                &&  this->anchortracktoplogy->signalsStatus["1101|LJ"]==0x01
                &&  this->anchortracktoplogy->signalsStatus["1101|HJ"]==0x01
                &&  this->anchortracktoplogy->signalsStatus["1101|LUJ"]==0x01
                &&  this->anchortracktoplogy->signalsStatus["1101|UJ"]==0x00
                ) {
                    currentStatusof1101="U";
                  }


        if(this->anchortracktoplogy->signalsStatus["1101|LUJ"]==0x00
                && this->anchortracktoplogy->signalsStatus["1101|UJ"]==0x01
                &&  this->anchortracktoplogy->signalsStatus["1101|HJ"]==0x01
                &&  this->anchortracktoplogy->signalsStatus["1101|LJ"]==0x01
                &&  this->anchortracktoplogy->signalsStatus["1101|2DJ"]==0x00
                &&  this->anchortracktoplogy->signalsStatus["1101|DJ"]==0x00
         ) {
                currentStatusof1101="LU";
        }

        if(this->anchortracktoplogy->signalsStatus["1101|LUJ"]==0x01
                && this->anchortracktoplogy->signalsStatus["1101|UJ"]==0x01
                &&  this->anchortracktoplogy->signalsStatus["1101|HJ"]==0x01
                &&  this->anchortracktoplogy->signalsStatus["1101|LJ"]==0x00
                &&  this->anchortracktoplogy->signalsStatus["1101|2DJ"]==0x01
                &&  this->anchortracktoplogy->signalsStatus["1101|DJ"]==0x00
                ) {
                currentStatusof1101="L";
            }
        if(whichoperation=="H"){  //想要X1变红

            if(currentStatusof1101=="L"){
                tempba[2] = (driveX1colarbytes["H"] & drive1101colarbytes["L"]);

            }else if(currentStatusof1101=="LU"){
                tempba[2] = (driveX1colarbytes["H"] & drive1101colarbytes["LU"]);
            }
            else if(currentStatusof1101=="U"){
                tempba[2] = (driveX1colarbytes["H"] & drive1101colarbytes["U"]);
            }
            else if(currentStatusof1101=="H"){
                tempba[2] = (driveX1colarbytes["H"] & drive1101colarbytes["H"]);
            }else{
                tempba[2] = 0xff & 0xfe;
            }
            return QByteArray::fromStdString(appendCRC(tempba));
        }
        if(whichoperation=="U"){

            QByteArray tempba;
            tempba.resize(3);
            tempba[0]=0x91;
            tempba[1]=0xff;
            if(currentStatusof1101=="L"){
                tempba[2] = (driveX1colarbytes["U"] & drive1101colarbytes["L"]);
            }else if(currentStatusof1101=="LU"){
                tempba[2] = (driveX1colarbytes["U"] & drive1101colarbytes["LU"]);
            }
            else if(currentStatusof1101=="U"){
                tempba[2] = (driveX1colarbytes["U"] & drive1101colarbytes["U"]);
            }
            else if(currentStatusof1101=="H"){
                tempba[2] = (driveX1colarbytes["U"] & drive1101colarbytes["H"]);
            }else{
                tempba[2] = 0xff & 0xfc;
            }
            return QByteArray::fromStdString(appendCRC(tempba));
        }

        if(whichoperation=="LU"){
            QByteArray tempba;
            tempba.resize(3);
            tempba[0]=0x91;
            tempba[1]=0xff;
            if(currentStatusof1101=="L"){
                tempba[2] = (driveX1colarbytes["LU"] & drive1101colarbytes["L"]);
            }else if(currentStatusof1101=="LU"){
                tempba[2] = (driveX1colarbytes["LU"] & drive1101colarbytes["LU"]);
            }
            else if(currentStatusof1101=="U"){
                tempba[2] =(driveX1colarbytes["LU"] & drive1101colarbytes["U"]);
            }
            else if(currentStatusof1101=="H"){
                tempba[2] = (driveX1colarbytes["LU"] & drive1101colarbytes["H"]);
            }else{
                tempba[2] = 0xff & 0xf4;
            }
            return QByteArray::fromStdString(appendCRC(tempba));
        }
        if(whichoperation=="L"){
            QByteArray tempba;
            tempba.resize(3);
            tempba[0]=0x91;
            tempba[1]=0xff;
            if(currentStatusof1101=="L"){
                tempba[2] = (driveX1colarbytes["L"] & drive1101colarbytes["L"]);
            }else if(currentStatusof1101=="LU"){
                tempba[2] = (driveX1colarbytes["L"] & drive1101colarbytes["LU"]);
            }
            else if(currentStatusof1101=="U"){
                tempba[2] = (driveX1colarbytes["L"] & drive1101colarbytes["U"]);
            }
            else if(currentStatusof1101=="H"){
                tempba[2] = (driveX1colarbytes["L"] & drive1101colarbytes["H"]);
            }else{
                tempba[2] = 0xff & 0xf8;
            }
            return QByteArray::fromStdString(appendCRC(tempba));
        }
    }
    //当1101HJ==0 && 1101DJ==0的时候表示正向红灯，    1101是红灯
    if(whichsignal=="1101"){

        //获取X1状态
        if(this->anchortracktoplogy->signalsStatus["X1|DJ"]==0x00
            &&this->anchortracktoplogy->signalsStatus["X1|LUJ"]==0x01
            &&this->anchortracktoplogy->signalsStatus["X1|LJ"]==0x01
            &&this->anchortracktoplogy->signalsStatus["X1|LXJ"]==0x01
                ) {
            currentStatusofX1="H";
        }
        if(this->anchortracktoplogy->signalsStatus["X1|DJ"]==0x00
                &&this->anchortracktoplogy->signalsStatus["X1|LUJ"]==0x01
                &&this->anchortracktoplogy->signalsStatus["X1|LJ"]==0x01
                &&this->anchortracktoplogy->signalsStatus["X1|LXJ"]==0x00

                ) {
            currentStatusofX1="U";
        }

        if(this->anchortracktoplogy->signalsStatus["X1|DJ"]==0x00
                &&this->anchortracktoplogy->signalsStatus["X1|LUJ"]==0x00
                &&this->anchortracktoplogy->signalsStatus["X1|LJ"]==0x01
                &&this->anchortracktoplogy->signalsStatus["X1|LXJ"]==0x00) {
            currentStatusofX1="LU";
        }

        if(this->anchortracktoplogy->signalsStatus["X1|DJ"]==0x00
                &&this->anchortracktoplogy->signalsStatus["X1|LUJ"]==0x01
                &&this->anchortracktoplogy->signalsStatus["X1|LJ"]==0x00
                &&this->anchortracktoplogy->signalsStatus["X1|LXJ"]==0x00) {
            currentStatusofX1="L";
        }

        if(whichoperation=="H"){
            QByteArray tempba;
            tempba.resize(3);
            tempba[0]=0x91;
            tempba[1]=0xff;
            if(currentStatusofX1=="L"){ //想要1101变红
                tempba[2] = (driveX1colarbytes["L"] &  drive1101colarbytes["H"]);
            }else if(currentStatusofX1=="LU"){
                tempba[2] = (driveX1colarbytes["LU"] &  drive1101colarbytes["H"]);
            }
            else if(currentStatusofX1=="U"){
                tempba[2] = (driveX1colarbytes["U"] &  drive1101colarbytes["H"]);
            }
            else if(currentStatusofX1=="H"){
                tempba[2] = (driveX1colarbytes["H"] &  drive1101colarbytes["H"]);
            }else{
                tempba[2] = 0xff & 0xdf;
            }

            return QByteArray::fromStdString(appendCRC(tempba));
        }
        if(whichoperation=="U"){
            QByteArray tempba;
            tempba.resize(3);
            tempba[0]=0x91;
            tempba[1]=0xff;
            if(currentStatusofX1=="L"){
                tempba[2] = (driveX1colarbytes["L"] &  drive1101colarbytes["U"]);
            }else if(currentStatusofX1=="LU"){
                tempba[2] = (driveX1colarbytes["LU"] &  drive1101colarbytes["U"]);
            }
            else if(currentStatusofX1=="U"){
                tempba[2] = (driveX1colarbytes["U"] &  drive1101colarbytes["U"]);
            }
            else if(currentStatusofX1=="H"){
                tempba[2] = (driveX1colarbytes["H"] &  drive1101colarbytes["U"]);
            }else{
                tempba[2] = 0xff & 0xbf;
            }
            return QByteArray::fromStdString(appendCRC(tempba));
        }

        if(whichoperation=="LU"){
            QByteArray tempba;
            tempba.resize(3);
            tempba[0]=0x91;
            tempba[1]=0xff;
            if(currentStatusofX1=="L"){
                tempba[2] = (driveX1colarbytes["L"] &  drive1101colarbytes["LU"]);
            }else if(currentStatusofX1=="LU"){
                tempba[2] = (driveX1colarbytes["LU"] &  drive1101colarbytes["LU"]);
            }
            else if(currentStatusofX1=="U"){
                tempba[2] = (driveX1colarbytes["U"] &  drive1101colarbytes["LU"]);
            }
            else if(currentStatusofX1=="H"){
                tempba[2] = (driveX1colarbytes["H"] &  drive1101colarbytes["LU"]);
            }else{
                tempba[2] = 0xff & 0x7f;
            }
            return QByteArray::fromStdString(appendCRC(tempba));
        }
        if(whichoperation=="L"){
            QByteArray tempba;
            tempba.resize(3);
            tempba[0]=0x91;
            tempba[1]=0xff;
            if(currentStatusofX1=="L"){
                tempba[2] = (driveX1colarbytes["L"] &  drive1101colarbytes["L"]);
            }else if(currentStatusofX1=="LU"){
                tempba[2] = (driveX1colarbytes["LU"] &  drive1101colarbytes["L"]);
            }
            else if(currentStatusofX1=="U"){
                tempba[2] = (driveX1colarbytes["U"] &  drive1101colarbytes["L"]);
            }
            else if(currentStatusofX1=="H"){
                tempba[2] = (driveX1colarbytes["H"] &  drive1101colarbytes["L"]);
            }else{
                tempba[2] = 0xff & 0xef;
            }
            return QByteArray::fromStdString(appendCRC(tempba));
        }
    }
    return NULL;
}

unsigned char ToplogyScene::convertStrToByte(QString str){
    int tempain=0;
    int j=0;
    for(int i=7;i>=0;i--){
        tempain = tempain + (str.at(j)=='1'? 1*qPow(2,i):0);
        j=j+1;
    }
    return  (unsigned char)tempain;
}

QByteArray ToplogyScene::driver1102110411061108command(QString whichsignal,QString whichoperation){

    QString data1="11";  //数据位前两位为11
    QString data2="11";
    if(this->anchortracktoplogy->signalsStatus.contains("1106|L")==false){
        return NULL;
    }
    data1+=(this->anchortracktoplogy->signalsStatus["1108|U"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["1108|H"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["1108|L"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["1106|U"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["1106|H"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["1106|L"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1104|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1104|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1104|L"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1102|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1102|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1102|L"]==0x00)?"0":"1";
    auto firstbyte = this->convertStrToByte(data1);
    auto secondbyte = this->convertStrToByte(data2);
    QString key = whichsignal+"-"+whichoperation;
    auto tempvalue = qudongzhilingmap.value(0x97,"");
    if(tempvalue.length()>1){
        unsigned char finalbyte1 =0xff;
        unsigned char finalbyte2 =0xff;
        auto zz = tempvalue.split('|');
        int i=0;
        for(i=0;i<zz.size();i++){
            if(zz[i]==key){
                break;
            }
        }
        if(i<=7){
            finalbyte1 = firstbyte & bit2colorcolor.value(i);
        }else{
            finalbyte2 = secondbyte & bit2colorcolor.value(i);
        }
        QByteArray tempba;
        tempba.resize(3);
        tempba[0]=0x97;
        tempba[1]=finalbyte1;
        tempba[2] =finalbyte2;
        return QByteArray::fromStdString(appendCRC(tempba));
    }
}



QByteArray ToplogyScene::driver1103ColoarComand(QString colorinfo){

    QString data1="11111";  //数据位前两位为11
    QString data2="11";

    data1+=(this->anchortracktoplogy->signalsStatus["1107|U"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["1107|H"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["1107|L"]==0x00)?"0":"1";


    data2+=(this->anchortracktoplogy->signalsStatus["1105|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1105|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1105|L"]==0x00)?"0":"1";

    if(colorinfo=="H"){
        data2+="101";
    } else if (colorinfo=="LU"){
        data2+="010";
    }else if(colorinfo=="UL"){
        data2+="010";
    }else if(colorinfo=="L"){
        data2+="110";
    }else if(colorinfo=="U"){
        data2+="011";
    }else {
      data2+="111";
    }

    auto firstbyte = this->convertStrToByte(data1);
    auto secondbyte = this->convertStrToByte(data2);
    QByteArray tempba;
    tempba.resize(3);
    tempba[0]=0x96;
    tempba[1]=firstbyte;
    tempba[2] =secondbyte;
    return QByteArray::fromStdString(appendCRC(tempba));
}




QByteArray ToplogyScene::driver1105ColoarComand(QString colorinfo){

    QString data1="11111";  //数据位前两位为11
    QString data2="11";
    data1+=(this->anchortracktoplogy->signalsStatus["1107|U"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["1107|H"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["1107|L"]==0x00)?"0":"1";
    if(colorinfo=="H"){
        data2+="101";
    } else if (colorinfo=="LU"){
        data2+="010";
    }else if(colorinfo=="UL"){
        data2+="010";
    }else if(colorinfo=="L"){
        data2+="110";
    }else if(colorinfo=="U"){
        data2+="011";
    }else {
      data2+="111";
    }
    data2+=(this->anchortracktoplogy->signalsStatus["1103|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1103|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1103|L"]==0x00)?"0":"1";
    auto firstbyte = this->convertStrToByte(data1);
    auto secondbyte = this->convertStrToByte(data2);
    QByteArray tempba;
    tempba.resize(3);
    tempba[0]=0x96;
    tempba[1]=firstbyte;
    tempba[2] =secondbyte;
    return QByteArray::fromStdString(appendCRC(tempba));
}



QByteArray ToplogyScene::driverXColorCommand(QString colorinfo){

    QString data1="11";  //数据位前两位为11
    data1+=(this->anchortracktoplogy->signalsStatus["S1|FB"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["S1|U"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["S1|H"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["S1|L"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["XN|YB"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["XN|2U"]==0x00)?"0":"1";
    QString data2="";  //数据位前两位为11
    data2+=(this->anchortracktoplogy->signalsStatus["XN|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["XN|L"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["XN|U"]==0x00)?"0":"1";

    if(colorinfo=="U"){
         data2+="11110";
    }else if(colorinfo=="L"){
         data2+="11101";
    }else if(colorinfo=="H"){
        data2+="11011";
    }else if(colorinfo=="2U"){
        data2+="10111";
    }else if(colorinfo=="LU"){
        data2+="11100";
    }else if(colorinfo=="UL"){
        data2+="11100";
    }else if(colorinfo=="YB"){
        data2+="01111";
    }else{
        data2+=(this->anchortracktoplogy->signalsStatus["X|YB"]==0x00)?"0":"1";
        data2+=(this->anchortracktoplogy->signalsStatus["X|2U"]==0x00)?"0":"1";
        data2+=(this->anchortracktoplogy->signalsStatus["X|H"]==0x00)?"0":"1";
        data2+=(this->anchortracktoplogy->signalsStatus["X|L"]==0x00)?"0":"1";
        data2+=(this->anchortracktoplogy->signalsStatus["X|U"]==0x00)?"0":"1";
    }

    auto firstbyte = this->convertStrToByte(data1);
    auto secondbyte = this->convertStrToByte(data2);
    QByteArray tempba;
    tempba.resize(3);
    tempba[0]=0x96;
    tempba[1]=firstbyte;
    tempba[2] =secondbyte;
    return QByteArray::fromStdString(appendCRC(tempba));
}


QByteArray ToplogyScene::driver1107ColoarComand(QString colorinfo){

    QString data1="11111";  //数据位前两位为11
    QString data2="11";

    if(colorinfo=="H"){
        data1+="101";
    } else if (colorinfo=="LU"){
        data1+="010";
    }else if(colorinfo=="UL"){
        data1+="010";
    }else if(colorinfo=="L"){
        data1+="110";
    }else if(colorinfo=="U"){
        data1+="011";
    }else {
      data1+="111";
    }

    data2+=(this->anchortracktoplogy->signalsStatus["1105|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1105|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1105|L"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1103|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1103|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1103|L"]==0x00)?"0":"1";
    auto firstbyte = this->convertStrToByte(data1);
    auto secondbyte = this->convertStrToByte(data2);
    QByteArray tempba;
    tempba.resize(3);
    tempba[0]=0x96;
    tempba[1]=firstbyte;
    tempba[2] =secondbyte;
    return QByteArray::fromStdString(appendCRC(tempba));
}



QByteArray ToplogyScene::driver110311051107command(QString whichsignal,QString whichoperation){


    //qudongzhilingmap.insert(0x96,"-|-|-|-|-|1107-U|1107-H|1107-L|-|-|1105-U|1105-H|1105-L|1103-U|1103-H|1103-L");
    QString data1="11111";  //数据位前两位为11
    QString data2="11";
    if(this->anchortracktoplogy->signalsStatus.contains("1107|L")==false){
        return NULL;
    }
    data1+=(this->anchortracktoplogy->signalsStatus["1107|U"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["1107|H"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["1107|L"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1105|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1105|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1105|L"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1103|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1103|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1103|L"]==0x00)?"0":"1";
    auto firstbyte = this->convertStrToByte(data1);
    auto secondbyte = this->convertStrToByte(data2);
    QString key = whichsignal+"-"+whichoperation;
    auto tempvalue = qudongzhilingmap.value(0x96,"");
    if(tempvalue.length()>1){
        unsigned char finalbyte1 =0xff;
        unsigned char finalbyte2 =0xff;
        auto zz = tempvalue.split('|');
        int i=0;
        for(i=0;i<zz.size();i++){
            if(zz[i]==key){
                break;
            }
        }
        if(i<=7){
            finalbyte1 = firstbyte & bit2colorcolor.value(i);
        }else{
            finalbyte2 = secondbyte & bit2colorcolor.value(i);
        }
        QByteArray tempba;
        tempba.resize(3);
        tempba[0]=0x96;
        tempba[1]=finalbyte1;
        tempba[2] =finalbyte2;
        return QByteArray::fromStdString(appendCRC(tempba));
    }
}


//设置信号灯SN的各种颜色
QByteArray ToplogyScene::driverSNColoarComand(QString colorinfo){

    QString data1="1";  //数据位前两位为11
    QString data2="11";

    data1+=(this->anchortracktoplogy->signalsStatus["X4|H"]==0x00)?"0":"1";

    if(colorinfo=="H"){
        data1+="101";
    } else if (colorinfo=="LU"){
        data1+="010";
    }else if(colorinfo=="UL"){
        data1+="010";
    }else if(colorinfo=="L"){
        data1+="110";
    }else if(colorinfo=="U"){
        data1+="011";
    }else {
      data1+="111";
    }

    data2+=(this->anchortracktoplogy->signalsStatus["1105|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1105|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1105|L"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1103|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1103|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["1103|L"]==0x00)?"0":"1";
    auto firstbyte = this->convertStrToByte(data1);
    auto secondbyte = this->convertStrToByte(data2);
    QByteArray tempba;
    tempba.resize(3);
    tempba[0]=0x96;
    tempba[1]=firstbyte;
    tempba[2] =secondbyte;
    return QByteArray::fromStdString(appendCRC(tempba));

}



QByteArray ToplogyScene::driverSSNX3X4command(QString whichsignal, QString whichoperation){

    QString data1="1";  //数据位前两位为11
    QString data2="";

    if(this->anchortracktoplogy->signalsStatus.contains("SN|2U")==false){
        return NULL;
    }
    data1+=(this->anchortracktoplogy->signalsStatus["X4|H"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["X4|L"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["X3|U"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["X3|H"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["X3|L"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["SN|YB"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["SN|2U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["SN|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["SN|L"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["SN|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["S|YB"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["S|2U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["S|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["S|L"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["S|U"]==0x00)?"0":"1";
    auto firstbyte = this->convertStrToByte(data1);
    auto secondbyte = this->convertStrToByte(data2);
    QString key = whichsignal+"-"+whichoperation;
    auto tempvalue = qudongzhilingmap.value(0x95,"");
    if(tempvalue.length()>1){
        unsigned char finalbyte1 =0xff;
        unsigned char finalbyte2 =0xff;
        auto zz = tempvalue.split('|');
        int i=0;
        for(i=0;i<zz.size();i++){
            if(zz[i]==key){
                break;
            }
        }
        if(i<=7){
            finalbyte1 = firstbyte & bit2colorcolor.value(i);
        }else{
            finalbyte2 = secondbyte & bit2colorcolor.value(i);
        }
        QByteArray tempba;
        tempba.resize(3);
        tempba[0]=0x95;
        tempba[1]=finalbyte1;
        tempba[2] =finalbyte2;
        return QByteArray::fromStdString(appendCRC(tempba));
    }
}

QByteArray ToplogyScene::driverS2S3S4X2command(QString whichsignal, QString whichoperation){


    QString data1="11";  //数据位前两位为11
    QString data2="";

    if(this->anchortracktoplogy->signalsStatus.contains("S4|L")==false){
        return NULL;
    }
    data1+=(this->anchortracktoplogy->signalsStatus["XII|H"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["XII|L"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["S4|FB"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["S4|U"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["S4|H"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["S4|L"]==0x00)?"0":"1";



    data2+=(this->anchortracktoplogy->signalsStatus["S3|FB"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["S3|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["S3|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["S3|L"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["SII|FB"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["SII|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["SII|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["SII|L"]==0x00)?"0":"1";


    auto firstbyte = this->convertStrToByte(data1);
    auto secondbyte = this->convertStrToByte(data2);

    QString key = whichsignal+"-"+whichoperation;
    auto tempvalue = qudongzhilingmap.value(0x94,"");

    if(tempvalue.length()>1){

        unsigned char finalbyte1 =0xff;
        unsigned char finalbyte2 =0xff;
        auto zz = tempvalue.split('|');
        int i=0;
        for(i=0;i<zz.size();i++){
            if(zz[i]==key){
                break;
            }
        }
        if(i<=7){
            finalbyte1 = firstbyte & bit2colorcolor.value(i);
        }else{
            finalbyte2 = secondbyte & bit2colorcolor.value(i);
        }
        QByteArray tempba;
        tempba.resize(3);
        tempba[0]=0x94;
        tempba[1]=finalbyte1;
        tempba[2] =finalbyte2;
        return QByteArray::fromStdString(appendCRC(tempba));


    }
}


QByteArray ToplogyScene::driverXXNS1command(QString whichsignal,QString whichoperation){
    QString data1="11";  //数据位前两位为11
    QString data2="";

    if(this->anchortracktoplogy->signalsStatus.contains("XN|2U")==false){
        return NULL;
    }
    data1+=(this->anchortracktoplogy->signalsStatus["S1|FB"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["S1|U"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["S1|H"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["S1|L"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["XN|YB"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->signalsStatus["XN|2U"]==0x00)?"0":"1";

    data2+=(this->anchortracktoplogy->signalsStatus["XN|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["XN|L"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["XN|U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["X|YB"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["X|2U"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["X|H"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["X|L"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->signalsStatus["X|U"]==0x00)?"0":"1";


    auto firstbyte = this->convertStrToByte(data1);
    auto secondbyte = this->convertStrToByte(data2);

    QString key = whichsignal+"-"+whichoperation;
    auto tempvalue = qudongzhilingmap.value(0x93,"");

    if(tempvalue.length()>1){

        unsigned char finalbyte1 =0xff;
        unsigned char finalbyte2 =0xff;
        auto zz = tempvalue.split('|');
        int i=0;
        for(i=0;i<zz.size();i++){
            if(zz[i]==key){
                break;
            }
        }
        if(i<=7){
            finalbyte1 = firstbyte & bit2colorcolor.value(i);
        }else{
            finalbyte2 = secondbyte & bit2colorcolor.value(i);
        }
        QByteArray tempba;
        tempba.resize(3);
        tempba[0]=0x93;
        tempba[1]=finalbyte1;
        tempba[2] =finalbyte2;
        return QByteArray::fromStdString(appendCRC(tempba));

    }

}


QByteArray ToplogyScene::driver13579112command(QString whichswitch,QString whichoperation){

    //
    QString data1="11";  //数据位前两位为11
    QString data2="";
    if(this->anchortracktoplogy->switchsStatus.contains("2|FB")==false){
        return NULL;
    }
    data1+=(this->anchortracktoplogy->switchsStatus["2|FB"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->switchsStatus["2|DB"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->switchsStatus["11|FB"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->switchsStatus["11|DB"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->switchsStatus["9|FB"]==0x00)?"0":"1";
    data1+=(this->anchortracktoplogy->switchsStatus["9|DB"]==0x00)?"0":"1";

    data2+=(this->anchortracktoplogy->switchsStatus["7|FB"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->switchsStatus["7|DB"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->switchsStatus["5|FB"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->switchsStatus["5|DB"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->switchsStatus["3|FB"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->switchsStatus["3|DB"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->switchsStatus["1|FB"]==0x00)?"0":"1";
    data2+=(this->anchortracktoplogy->switchsStatus["1|DB"]==0x00)?"0":"1";

    auto firstbyte = this->convertStrToByte(data1);
    auto secondbyte = this->convertStrToByte(data2);
    QString key = whichswitch+"#"+whichoperation;
    auto tempvalue = qudongzhilingmap.value(0x92,"");

    if(tempvalue.length()>1){

        unsigned char finalbyte1 =0xff;
        unsigned char finalbyte2 =0xff;
        auto zz = tempvalue.split('|');
        int i=0;
        for(i=0;i<zz.size();i++){
            if(zz[i]==key){
                break;
            }
        }
        if(i<=7){
            finalbyte1 = firstbyte & bit2colorcolor.value(i);
        }else{
            finalbyte2 = secondbyte & bit2colorcolor.value(i);
        }
        QByteArray tempba;
        tempba.resize(3);
        tempba[0]=0x92;
        tempba[1]=finalbyte1;
        tempba[2] =finalbyte2;
        return QByteArray::fromStdString(appendCRC(tempba));


    }
}


QByteArray ToplogyScene::driver4daocha(QString dfc){

    unsigned char switch2byte_4_DC = 0xeb;
    unsigned char switch2byte_4_FC = 0xe7;
    QByteArray temp_data7;
    temp_data7.resize(3);
    temp_data7[0] = 0x90;           //驱动非实物转辙机定操
    temp_data7[1] = 0xff;
    if(dfc=="DC"){
        temp_data7[2] = 0xff & switch2byte_4_DC;
    }else{
        temp_data7[2] = 0xff & switch2byte_4_FC;
    }
    return QByteArray::fromStdString(appendCRC(temp_data7));

}



void ToplogyScene::pushCommanItemToQueue(QByteArray ba){
    if(this->localcomandqueue.size()<10){
        localcomandqueue.append(ba);
    }else{
        //轨道电路的发送命令优先级大于0xfa,0x91,0x92,0x93,0xc0,0xc1,0xc2
        //qDebug()<<"运行到.....pushCommanItemToQueue...";
        QVector<CommmandClass> vec;
        //初始化数据
        while (!localcomandqueue.isEmpty())
        {
            auto zz = localcomandqueue.dequeue();
            unsigned char tempkey =zz.at(0);
            CommmandClass temclasscommand;
            temclasscommand.id = tempkey;
            temclasscommand.ba =zz;
            //vec.push_back(temclasscommand);
            if(temclasscommand.id==0xfa){
                commanditems.enqueue(temclasscommand.ba.toStdString());
            }else{
                vec.push_back(temclasscommand);
            }
        }

        std::sort(vec.begin(),vec.end(),compare);
        for(auto it:vec){
            commanditems.enqueue(it.ba.toStdString());
        }
    }
}

//在前方有多少个可用区段
int ToplogyScene::getHowManyAccessibleSegmentsAhead(QList<QString> signalss){
    int aviableSegmentscount=0;
    for(int i=0;i<signalss.size();i++){
        auto zz =signalss.at(i);
        auto signal1= zz+"-B1";
        auto signal2= zz+"-B2";
        auto color1 = this->signal2Color.contains(signal1)?this->signal2Color[signal1]:"black";
        auto color2 = this->signal2Color.contains(signal2)?this->signal2Color[signal2]:"black";
        if(color1=="red" || color2=="red"){
            return aviableSegmentscount;
        }else{
            if(color1=="black" && color2=="black"){
                continue;
            }else{
                aviableSegmentscount=aviableSegmentscount+1;
            }
        }
    }
    return aviableSegmentscount;
}

//获取哪一个信号灯保护该区段
QString ToplogyScene::getSignalBySegmentprotected(QString protected_segement){


    QMap<QString, QString>::iterator iter = signal_prote_segement.begin();
    while (iter != signal_prote_segement.end())
    {
        if(iter.value().contains("|")){
            auto zz=iter.value().split('|');
            auto tmps1 = zz.value(0,"");
            auto tmps2 = zz.value(1,"");
            if(tmps1==protected_segement || tmps2==protected_segement){
                return iter.key();
            }
        }else{
            if(iter.value()==protected_segement){
                return iter.key();
            }
        }
        iter++;
    }

    return "";     //没有就返回这个
}

QString  ToplogyScene::generateFaMaSerial(QString segment_name){

    QMap<int,QString> fama2code;
    fama2code.insert(0,"HU");
    fama2code.insert(1,"U");
    fama2code.insert(2,"LU");
    fama2code.insert(3,"L");
    fama2code.insert(4,"L2");
    fama2code.insert(5,"L3");

    int pos=0;
    auto tempbool= segmentlist->locateLNode(pos,segment_name);
    QString ma="JC";
    if(tempbool==true){
        QVector<int> temparr;        //区段对应的在循环列表中的序号
        for(int i=1;i<=segmentlist->countNode-1;i++){
            auto zz= pos+i;
            if(zz>segmentlist->countNode){         //circly search the segmenet ahead
                temparr.append(zz%(segmentlist->countNode));
            }else{
                temparr.append(zz);
            }
        }
        QList<QString> signalss; //所有保护这些区段的信号灯
        //获取在前面有多少个可用分段
        for(int i=0;i<temparr.size();i++){
            auto tmpssssss = segmentlist->getNodeDataByPosition(temparr.at(i));
            auto tmpzz =getSignalBySegmentprotected(tmpssssss);
            signalss.append(tmpzz);
        }
        signalss.removeDuplicates();
        //根据防护信号寻找空闲区段个数
        int avaibleSegments=getHowManyAccessibleSegmentsAhead(signalss);
        qDebug()<<"在该进路前方空闲区段......."<<avaibleSegments;
        //在求第4DG的时候有点特殊

        if(avaibleSegments<0 && avaibleSegments>5){
            ma="JC";
        }else{
            if(avaibleSegments==0 && segment_name=="4DG"){
                //4DG要特殊处理一下
                ma ="JC";
            }else{
                ma =fama2code.value(avaibleSegments);
            }
        }
    }
    return ma;
}


void ToplogyScene::clearButtonClickedEventAndReserverPath(QString startx, QString endx){

    try{
        while(!buttonclickedvents.isEmpty())
        {
            auto r =buttonclickedvents.pop();//出栈,注意出栈后，栈仍然保持，因此额外做一次清除操作
            emit shutdownTrainsignal(r); //按钮X-A1灭灯,在灭灯之后
        }
        buttonclickedvents.clear();
    }
    catch(...){
        this->anchortracktoplogy->ReserverRoutePath("harmony",startx,endx);
        initialfamacomand();
    }
}

void ToplogyScene::TryRouterPermitAndDrwaScene(QString startx, QString endx){
    if(startx=="X" && endx=="SI"){
        int dcarray[]={1,3,5,7,11};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }
        if(anchortracktoplogy->cijiaSignalStatus["XI-B1"]=="red" && anchortracktoplogy->cijiaSignalStatus["XI-B2"]==""){
            qDebug()<<"1-1 进路情况.....................";
            emit drawlineSignal("X","XI","white");
            emit signalShouldDisplayStatus("X-B2","yellow");
            emit signalShouldDisplayStatus("X-B1","black");
            emit emitDriverCommand(this->driverXXNS1command("X","U"));
        }
        if(anchortracktoplogy->cijiaSignalStatus["XI-B1"]=="" && anchortracktoplogy->cijiaSignalStatus["XI-B2"]=="yellow"){
            qDebug()<<"1-2 进路情况.....................";
            emit drawlineSignal("X","SN","white");
            emit signalShouldDisplayStatus("X-B1","yellow");
            emit signalShouldDisplayStatus("X-B2","green");
            emit swithoperationsignaldisplay(QString::number(4),"directed");
            emit emitDriverCommand(this->driverXXNS1command("X","L"));
            emit emitDriverCommand(this->driverXXNS1command("X","U"));
        }
        if(anchortracktoplogy->cijiaSignalStatus["XI-B1"]=="yellow" && anchortracktoplogy->cijiaSignalStatus["XI-B2"]=="green"){
            qDebug()<<"1-3 进路情况.....................";
            emit drawlineSignal("X","SN","white");
            emit signalShouldDisplayStatus("X-B1","black");
            emit signalShouldDisplayStatus("X-B2","green");
            emit emitDriverCommand(this->driverXXNS1command("X","L"));
            emit swithoperationsignaldisplay(QString::number(4),"directed");
        }

        if(anchortracktoplogy->cijiaSignalStatus["XI-B1"]=="" && anchortracktoplogy->cijiaSignalStatus["XI-B2"]=="green"){
            qDebug()<<"1-4 进路情况.....................";
            emit drawlineSignal("X","SN","white");
            emit signalShouldDisplayStatus("X-B1","black");
            emit signalShouldDisplayStatus("X-B2","green");
            emit swithoperationsignaldisplay(QString::number(4),"directed");
            emit emitDriverCommand(this->driverXXNS1command("X","L"));
        }
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){  //再画绿线
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }
    if(startx=="X" && endx=="S3"){

        emit drawlineSignal("X","11","white");
        emit drawlineSignal("11","上行侧轨开始处","white");
        emit drawlineSignal("上行侧轨开始处","上行侧轨X3处","white");
        int dcarray[]={1,3,5,7};
        //这几个都是驱动定操
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }
        emit emitDriverCommand(driver13579112command(QString(11),"FC"));
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){  //再画绿线
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        emit swithoperationsignaldisplay(QString::number(11),"reversed");  //11这个应该变成黄色，后面再改
        emit signalShouldDisplayStatus("X-B2","yellow");
        emit signalShouldDisplayStatus("X-B1","yellow");
        emit emitDriverCommand(this->driverXXNS1command("X","2U"));
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }

    if(startx=="X" && endx=="SII"){

        emit drawlineSignal("X","5","white");
        emit drawlineSignal("5","7","white");
        emit drawlineSignal("7","9","white");
        emit drawlineSignal("9","XII","white");
        int dcarray[]={1,3,9};
        //这几个都是驱动定操
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }
        emit emitDriverCommand(driver13579112command(QString(5),"FC"));
        emit emitDriverCommand(driver13579112command(QString(7),"FC"));

        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){  //再画绿线
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        emit swithoperationsignaldisplay("5","reversed"); //在主界面上定位显示
        emit swithoperationsignaldisplay("7","reversed"); //在主界面上定位显示
        emit signalShouldDisplayStatus("X-B2","yellow");
        emit signalShouldDisplayStatus("X-B1","yellow");
        emit emitDriverCommand(this->driverXXNS1command("X","2U"));
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }

    if(startx=="X" && endx=="S4"){

        emit drawlineSignal("X","5","white");
        emit drawlineSignal("5","7","white");
        emit drawlineSignal("7","9","white");
        emit drawlineSignal("9","下行侧轨开始处","white");
        emit drawlineSignal("下行侧轨开始处","下行侧轨X4处","white");
        int dcarray[]={1,3,9};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }
        emit emitDriverCommand(driver13579112command(QString(5),"FC"));
        emit emitDriverCommand(driver13579112command(QString(7),"FC"));


        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){  //再画绿线
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        emit swithoperationsignaldisplay("5","reversed"); //在主界面上定位显示
        emit swithoperationsignaldisplay("7","reversed"); //在主界面上定位显示
        emit swithoperationsignaldisplay("9","directed"); //在主界面上定位显示
        emit signalShouldDisplayStatus("X-B2","yellow");
        emit signalShouldDisplayStatus("X-B1","yellow");
        emit emitDriverCommand(this->driverXXNS1command("X","2U"));     //we should not only update signal but also the switchs
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }

    if(startx=="XN" && endx=="SII"){

        emit drawlineSignal("XN","XII","white");

        int dcarray[]={1,3,5,7,9};
        //这几个都是驱动定操
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }

        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){  //再画绿线
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        int zzzzzzz=anchortracktoplogy->getSingalStatus("XII");
        qDebug()<<"XII这个信号灯的颜色是........."<<zzzzzzz;
        if(anchortracktoplogy->getSingalStatus("XII")==3){
            emit signalShouldDisplayStatus("XN-B2","yellow");
            emit emitDriverCommand(this->driverXXNS1command("XN","U"));     //we should not only update signal but also the switchs
        }else if(anchortracktoplogy->getSingalStatus("XII")==1){
            emit signalShouldDisplayStatus("XN-B2","green");
            emit emitDriverCommand(this->driverXXNS1command("XN","L"));     //we should not only update signal but also the switchs
        }else{
        }
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }


    if(startx=="XN" && endx=="S4"){

        emit drawlineSignal("XN","9","white");
        emit drawlineSignal("9","下行侧轨开始处","white");
        emit drawlineSignal("下行侧轨开始处","下行侧轨X4处","white");
        int dcarray[]={1,3,5,7};
        //这几个都是驱动定操
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }
        emit emitDriverCommand(driver13579112command(QString(9),"FC"));

        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){  //再画绿线
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        emit swithoperationsignaldisplay(QString::number(9),"reversed");
        emit signalShouldDisplayStatus("XN-B1","yellow");
        emit signalShouldDisplayStatus("XN-B2","yellow");
        emit emitDriverCommand(this->driverXXNS1command("XN","2U"));     //we should not only update signal but also the switchs
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }


    if(startx=="XN" && endx=="SI"){

        emit drawlineSignal("XN","1","white");
        emit drawlineSignal("1","3","white");
        emit drawlineSignal("3","11","white");
        emit drawlineSignal("11","11","white");

        int dcarray[]={5,7,11};
        //这几个都是驱动定操
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }
        emit emitDriverCommand(driver13579112command(QString(1),"FC"));
        emit emitDriverCommand(driver13579112command(QString(3),"FC"));

        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){  //再画绿线
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        emit swithoperationsignaldisplay(QString::number(1),"reversed");
        emit swithoperationsignaldisplay(QString::number(3),"reversed");
        emit signalShouldDisplayStatus("XN-B1","yellow");
        emit signalShouldDisplayStatus("XN-B2","yellow");
        emit emitDriverCommand(this->driverXXNS1command("XN","2U"));     //we should not only update signal but also the switchs
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }


    if(startx=="XN" && endx=="S3"){

        emit drawlineSignal("XN","1","white");
        emit drawlineSignal("1","3","white");
        emit drawlineSignal("3","11","white");
        emit drawlineSignal("11","上行侧轨开始处","white");
        emit drawlineSignal("上行侧轨开始处","上行侧轨X3处","white");
        int dcarray[]={5,7};
        //这几个都是驱动定操
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"FC"));
        }
        emit emitDriverCommand(driver13579112command(QString(1),"DC"));
        emit emitDriverCommand(driver13579112command(QString(3),"DC"));
        emit emitDriverCommand(driver13579112command(QString(11),"DC"));


        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){  //再画绿线
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        emit swithoperationsignaldisplay(QString::number(1),"reversed");
        emit swithoperationsignaldisplay(QString::number(3),"reversed");
        emit swithoperationsignaldisplay(QString::number(11),"reversed");
        emit signalShouldDisplayStatus("XN-B1","yellow");
        emit signalShouldDisplayStatus("XN-B2","yellow");
        emit emitDriverCommand(this->driverXXNS1command("XN","2U"));     //we should not only update signal but also the switchs
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }


    if(startx=="S" && endx=="XII"){

        int dcarray[]={2};
        //这几个都是驱动定操
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }
        qDebug()<<"运行到这里S。。。。。。XII"<<anchortracktoplogy->getSingalStatus("SII");
        if(anchortracktoplogy->getSingalStatus("SII")==3){
            emit drawlineSignal("SII","S","white");
            //emit drawlineSignal("SII","XII","white");
            emit signalShouldDisplayStatus("S-B1","black");
            emit signalShouldDisplayStatus("S-B2","yellow");
            emit emitDriverCommand(this->driverSSNX3X4command("S","U"));     //we should not only update signal but also the switchs

        }

        if(anchortracktoplogy->getSingalStatus("SII")==2){
            emit drawlineSignal("XN","S","white");
            emit signalShouldDisplayStatus("S-B1","black");
            emit signalShouldDisplayStatus("S-B2","yellow");
            emit emitDriverCommand(this->driverSSNX3X4command("S","U"));     //we should not only update signal but also the switchs

        }


        if(anchortracktoplogy->getSingalStatus("SII")==1){
            emit drawlineSignal("XN","S","white");
            emit signalShouldDisplayStatus("S-B1","black");
            emit signalShouldDisplayStatus("S-B2","green");
            emit emitDriverCommand(this->driverSSNX3X4command("S","L"));     //we should not only update signal but also the switchs

        }
        emit swithoperationsignaldisplay(QString::number(2),"directed");
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }


    if(startx=="S" && endx=="X4"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        emit drawlineSignal("XN","9","white");
        emit drawlineSignal("9","下行侧轨开始处","white");
        emit drawlineSignal("下行侧轨开始处","下行侧轨X4处","white");
        int dcarray[]={2};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }
        emit signalShouldDisplayStatus("S-B1","yellow");
        emit signalShouldDisplayStatus("S-B2","yellow");
        emit emitDriverCommand(this->driverSSNX3X4command("S","2U"));     //we should not only update signal but also the switchs
        emit swithoperationsignaldisplay(QString::number(2),"reversed");
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }


    if(startx=="SN" && endx=="XI"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={4};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"FC"));
        }

        if(anchortracktoplogy->getSingalStatus("SI-B1")==3){
            emit drawlineSignal("SI","SN","white");
            emit signalShouldDisplayStatus("SN-B2","yellow");
            emit emitDriverCommand(this->driverSSNX3X4command("SN","U"));     //we should not only update signal but also the switchs

        }

        if(anchortracktoplogy->getSingalStatus("SI-B2")==1){
            emit drawlineSignal("X","SN","white");
            emit signalShouldDisplayStatus("SN-B2","green");
            emit emitDriverCommand(this->driverSSNX3X4command("SN","U"));     //we should not only update signal but also the switchs

        }
        emit swithoperationsignaldisplay(QString::number(4),"directed");
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }

    if(startx=="SN" && endx=="X3"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={4};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }

        emit signalShouldDisplayStatus("SN-B2","yellow");
        emit signalShouldDisplayStatus("SN-B1","yellow");
        emit emitDriverCommand(this->driverSSNX3X4command("SN","2U"));     //we should not only update signal but also the switchs
        emit drawlineSignal("S3","上行侧轨X3拖尾处","white");
        emit drawlineSignal("上行侧轨X3拖尾处","4","white");
        emit drawlineSignal("4","SN","white");
        emit swithoperationsignaldisplay(QString::number(4),"reversed");
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }

    if(startx=="XI" && endx=="SN"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={4};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"FC"));
        }

        if(anchortracktoplogy->getSingalStatus("1101")==3){
            emit drawlineSignal("SI","XN","white");
            emit signalShouldDisplayStatus("XI-B2","yellow");
            emit emitDriverCommand(driver1101_X1command("X1","U"));

        }

        if(anchortracktoplogy->getSingalStatus("1101")==2){
            emit drawlineSignal("SI","XN","white");
            emit signalShouldDisplayStatus("XI-B1","yellow");
            emit signalShouldDisplayStatus("XI-B2","green");
            emit emitDriverCommand(driver1101_X1command("X1","LU"));

        }

        if(anchortracktoplogy->getSingalStatus("1101")==1){
            emit drawlineSignal("SI","XN","white");
            emit signalShouldDisplayStatus("XI-B2","green");
            emit emitDriverCommand(driver1101_X1command("X1","L"));

        }

        emit swithoperationsignaldisplay(QString::number(4),"directed");
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }


    if(startx=="X3" && endx=="SN"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={4};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }

        if(anchortracktoplogy->getSingalStatus("1101")==3){
            emit signalShouldDisplayStatus("X3-B2","yellow");
            emit drawlineSignal("X3","上行侧轨X3拖尾处","white");
            emit drawlineSignal("上行侧轨X3拖尾处","4","white");
            emit drawlineSignal("4","SN","white");
            emit signalShouldDisplayStatus("X3-B2","yellow");
            emit emitDriverCommand(driverSSNX3X4command("X3","U"));
        }

        if(anchortracktoplogy->getSingalStatus("1101")==2){
            emit drawlineSignal("X3","上行侧轨X3拖尾处","white");
            emit drawlineSignal("上行侧轨X3拖尾处","4","white");
            emit drawlineSignal("4","SN","white");
            emit signalShouldDisplayStatus("X3-B1","yellow");
            emit signalShouldDisplayStatus("X3-B2","green");
            emit emitDriverCommand(driverSSNX3X4command("X3","U")); //
        }

        if(anchortracktoplogy->getSingalStatus("1101")==1){
            emit signalShouldDisplayStatus("X3-B2","green");
            emit drawlineSignal("X3","上行侧轨拖尾处","white");
            emit drawlineSignal("上行侧轨拖尾处","4","white");
            emit drawlineSignal("4","SN","white");
            emit emitDriverCommand(driverSSNX3X4command("X3","L")); //

        }

        emit swithoperationsignaldisplay(QString::number(4),"reversed");
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }

    if(startx=="XII" && endx=="S"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={2};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"FC"));
        }
        emit signalShouldDisplayStatus("XII-B2","green");
        emit emitDriverCommand(driverS2S3S4X2command("XII","L")); //
        emit drawlineSignal(startx,endx,"white");
        emit swithoperationsignaldisplay(QString::number(2),"directed");
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }


    if(startx=="X4" && endx=="S"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={2};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }

        emit signalShouldDisplayStatus("X4-B2","green");
        emit emitDriverCommand(driverSSNX3X4command("X4","L")); //
        emit drawlineSignal(startx,"下行侧轨X4拖尾处","white");
        emit drawlineSignal("下行侧轨X4拖尾处","2","white");
        emit drawlineSignal("2","S","white");
        emit swithoperationsignaldisplay(QString::number(2),"reversed");
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }



    if(startx=="SII" && endx=="XN"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={1,3,5,7,9};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"FC"));
        }

        if(anchortracktoplogy->getSingalStatus("1108")==3){
            emit drawlineSignal("XN","SII","white");
            emit signalShouldDisplayStatus("SII-B2","yellow");
            emit emitDriverCommand(driverS2S3S4X2command("SII","U"));
        }

        if(anchortracktoplogy->getSingalStatus("1108")==2){
            emit drawlineSignal("XN","SII","white");
            emit signalShouldDisplayStatus("SII-B1","yellow");
            emit signalShouldDisplayStatus("SII-B2","green");
            emit emitDriverCommand(driverS2S3S4X2command("SII","U"));  //I does not consider the double selection condition
            emit emitDriverCommand(driverS2S3S4X2command("SII","L"));  //this place should be carefully
        }

        if(anchortracktoplogy->getSingalStatus("1108")==1){
            emit drawlineSignal("XN","SII","white");
            emit signalShouldDisplayStatus("SII-B2","green");
            emit emitDriverCommand(driverS2S3S4X2command("SII","L"));  //this place should be carefully

        }
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }


    if(startx=="S4" && endx=="XN"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={1,3,5,7};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"FC"));
        }
        emit emitDriverCommand(driver13579112command(QString(9),"DC"));

        emit drawlineSignal("XN","9","white");
        emit drawlineSignal("9","下行侧轨开始处","white");
        emit drawlineSignal("下行侧轨开始处","S4","white");

        if(anchortracktoplogy->getSingalStatus("1108")==3){
            emit signalShouldDisplayStatus("S4-B2","yellow");
            emit emitDriverCommand(driverS2S3S4X2command("S4","U"));  //this place should be carefully
        }

        if(anchortracktoplogy->getSingalStatus("1108")==2){
            emit signalShouldDisplayStatus("S4-B1","yellow");
            emit signalShouldDisplayStatus("S4-B2","green");
            emit emitDriverCommand(driverS2S3S4X2command("S4","U"));  //this place should be carefully
            emit emitDriverCommand(driverS2S3S4X2command("S4","L"));  //this place should be carefully
        }

        if(anchortracktoplogy->getSingalStatus("1108")==1){
            emit signalShouldDisplayStatus("S4-B2","green");
            emit emitDriverCommand(driverS2S3S4X2command("S4","L"));  //this place should be carefully
        }
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        emit swithoperationsignaldisplay(QString::number(9),"reversed");

        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }

    if(startx=="SI" && endx=="XN"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={11,5,7};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"FC"));
        }
        emit emitDriverCommand(driver13579112command(QString(1),"DC"));
        emit emitDriverCommand(driver13579112command(QString(3),"DC"));

        emit drawlineSignal("XN","1","white");
        emit drawlineSignal("1","3","white");
        emit drawlineSignal("3","SI","white");

        if(anchortracktoplogy->getSingalStatus("1108")==3){

            emit signalShouldDisplayStatus("SI-B2","yellow");
            emit emitDriverCommand(driverXXNS1command("S1","U"));  //this place should be carefully


        }

        if(anchortracktoplogy->getSingalStatus("1108")==2){
            emit signalShouldDisplayStatus("SI-B1","yellow");
            emit signalShouldDisplayStatus("SI-B2","green");
            emit emitDriverCommand(driverXXNS1command("S1","U"));  //this place should be carefully
            emit emitDriverCommand(driverXXNS1command("S1","L"));  //this place should be carefully
        }

        if(anchortracktoplogy->getSingalStatus("1108")==1){
            emit signalShouldDisplayStatus("SI-B2","green");
            emit emitDriverCommand(driverXXNS1command("S1","L"));  //this place should be carefully

        }
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        emit swithoperationsignaldisplay(QString::number(1),"reversed");
        emit swithoperationsignaldisplay(QString::number(3),"reversed");

        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }


    if(startx=="S3" && endx=="XN"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={1,3,11};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }

        emit emitDriverCommand(driver13579112command(QString(5),"FC"));
        emit emitDriverCommand(driver13579112command(QString(7),"FC"));

        emit drawlineSignal("XN","1","white");
        emit drawlineSignal("1","3","white");
        emit drawlineSignal("3","11","white");
        emit drawlineSignal("11","上行侧轨开始处","white");
        emit drawlineSignal("上行侧轨开始处","S3","white");

        if(anchortracktoplogy->getSingalStatus("1108")==3){
            emit signalShouldDisplayStatus("S3-B2","yellow");
            emit emitDriverCommand(driverS2S3S4X2command("S3","U"));  //this place should be carefully

        }

        if(anchortracktoplogy->getSingalStatus("1108")==2){
            emit signalShouldDisplayStatus("S3-B1","yellow");
            emit signalShouldDisplayStatus("S3-B2","green");
            emit emitDriverCommand(driverS2S3S4X2command("S3","U"));  //this place should be carefully
            emit emitDriverCommand(driverS2S3S4X2command("S3","L"));  //this place should be carefully
        }

        if(anchortracktoplogy->getSingalStatus("1108")==1){
            emit signalShouldDisplayStatus("S3-B2","green");
            emit emitDriverCommand(driverS2S3S4X2command("S3","L"));  //this place should be carefully

        }
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"reversed");
        }
        emit swithoperationsignaldisplay(QString::number(5),"directed");
        emit swithoperationsignaldisplay(QString::number(7),"directed");

        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }



    if(startx=="SI" && endx=="X"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={1,3,5,7,11};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"FC"));
        }

        emit drawlineSignal(startx,endx,"white");
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        emit signalShouldDisplayStatus("SI-B2","green");
        emit emitDriverCommand(driverXXNS1command("S1","L"));  //this place should be carefully
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }


    if(startx=="S3" && endx=="X"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={1,3,5,7};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"FC"));
        }
        emit emitDriverCommand(driver13579112command(QString(11),"DC"));

        emit drawlineSignal("X","11","white");
        emit drawlineSignal("11","上行侧轨开始处","white");
        emit drawlineSignal("上行侧轨开始处","S3","white");


        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        emit swithoperationsignaldisplay(QString::number(11),"reversed");
        emit signalShouldDisplayStatus("S3-B2","green");
        emit emitDriverCommand(driverS2S3S4X2command("S3","L"));  //this place should be carefully
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }
    if(startx=="SII" && endx=="X"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={1,3,9};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }
        emit emitDriverCommand(driver13579112command(QString(5),"FC"));
        emit emitDriverCommand(driver13579112command(QString(7),"FC"));
        emit drawlineSignal("X","5","white");
        emit drawlineSignal("5","7","white");
        emit drawlineSignal("7","SII","white");
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        emit swithoperationsignaldisplay(QString::number(5),"reversed");
        emit swithoperationsignaldisplay(QString::number(7),"reversed");
        emit signalShouldDisplayStatus("SII-B2","green");
        emit emitDriverCommand(driverS2S3S4X2command("SII","L"));  //this place should be carefully
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }
    if(startx=="S4" && endx=="X"){
        //所有流程都是先画线、再驱动定操、再画定操、再设置灯的颜色、再清空按钮和保留进路
        int dcarray[]={1,3};
        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit emitDriverCommand(driver13579112command(QString(dcarray[i]),"DC"));
        }
        emit emitDriverCommand(driver13579112command(QString(5),"FC"));
        emit emitDriverCommand(driver13579112command(QString(7),"FC"));
        emit emitDriverCommand(driver13579112command(QString(9),"FC"));
        emit drawlineSignal("X","5","white");
        emit drawlineSignal("5","7","white");
        emit drawlineSignal("7","9","white");
        emit drawlineSignal("9","下行侧轨开始处","white");
        emit drawlineSignal("下行侧轨开始处","S4","white");

        for(int i=0;i<sizeof(dcarray) /sizeof(dcarray[0]);i++){
            emit swithoperationsignaldisplay(QString::number(dcarray[i]),"directed");
        }
        emit swithoperationsignaldisplay(QString::number(5),"reversed");
        emit swithoperationsignaldisplay(QString::number(7),"reversed");
        emit swithoperationsignaldisplay(QString::number(9),"reversed");
        emit signalShouldDisplayStatus("S4-B2","green");
        emit emitDriverCommand(driverS2S3S4X2command("S4","L"));  //this place should be carefully
        clearButtonClickedEventAndReserverPath(startx,endx);
        return;
    }
}

//格式都是X>Y
void ToplogyScene::RouteTakenHandler(QString startx,QString endx){
    this->anchortracktoplogy->fromStation="";
    this->anchortracktoplogy->toStation="";
    buttonclickedvents.clear();
}

void ToplogyScene::timerEvent(QTimerEvent *event){
    if(startupfaflag==0){
        emit signalShouldDisplayStatus("1107-B1","yellow");
        emit signalShouldDisplayStatus("1107-B2","black");
        emit signalShouldDisplayStatus("1105-B1","yellow");
        emit signalShouldDisplayStatus("1105-B2","green");
        emit signalShouldDisplayStatus("1103-B1","green");
        emit signalShouldDisplayStatus("1103-B2","black");
        emit signalShouldDisplayStatus("1101-B1","green");
        emit signalShouldDisplayStatus("1108-B1","green");
        emit signalShouldDisplayStatus("1106-B1","green");
        emit signalShouldDisplayStatus("1104-B1","yellow");
        emit signalShouldDisplayStatus("1104-B2","green");
        emit signalShouldDisplayStatus("1102-B1","yellow");
        emit signalShouldDisplayStatus("SN-B1","red");
        emit signalShouldDisplayStatus("S-B1","red");
        emit signalShouldDisplayStatus("X3-B1","red");
        emit signalShouldDisplayStatus("XI-B1","red");
        emit signalShouldDisplayStatus("XII-B1","red");
        emit signalShouldDisplayStatus("X4-B1","red");
        emit signalShouldDisplayStatus("S3-B1","red");
        emit signalShouldDisplayStatus("S3-B2","black");
        emit signalShouldDisplayStatus("SI-B1","red");
        emit signalShouldDisplayStatus("SI-B2","black");
        emit signalShouldDisplayStatus("SII-B1","red");
        emit signalShouldDisplayStatus("SII-B2","black");
        emit signalShouldDisplayStatus("S4-B1","red");
        emit signalShouldDisplayStatus("S4-B2","black");
        emit signalShouldDisplayStatus("X-B1","red");
        emit signalShouldDisplayStatus("X-B2","black");
        emit signalShouldDisplayStatus("XN-B1","red");
        emit signalShouldDisplayStatus("XN-B2","black");
        startupfaflag=1;
        guidaoSignalInitalStatus();
        initialfamacomand();
        guidaoSwitchIntialStatus();//初始化道岔状态

    }
    if(sendingchar<=0xc9){
        QByteArray ba;
        ba.resize(3);
        ba[0] = sendingchar;
        ba[1] = 0x80;
        ba[2] = 0x8e;
        this->pushCommanItemToQueue(ba);
        sendingchar = sendingchar+1;
    }else{
        sendingchar = 0xc0;
    }
}

ToplogyScene::~ToplogyScene()
{
    signallist->destroySelf();
    segmentlist->destroySelf();
}

void ToplogyScene::requestRouteHandler(QString item_name){
    this->anchortracktoplogy->requestRouteHandler(item_name);
}

void ToplogyScene::cacelrequeRouteHanler(QString item_name){
    qDebug()<<"in cacel request handler is true......"<<item_name;
    this->anchortracktoplogy->cacelRouteStartpointHanler(item_name);
}
void ToplogyScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);
    if(m_pItemSelected != nullptr){
        if(m_bPressed){
            m_Offset = event->scenePos() - m_Pressed; //鼠标移动的偏移量
        }
    }
}

void ToplogyScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{

    QGraphicsScene::mouseReleaseEvent(event);
    m_bPressed = false;
    if(m_pItemSelected != nullptr){
        m_pItemSelected = nullptr; //鼠标释放时将选中的Item设为空
    }
}
void ToplogyScene::DelaySpecifyTimePeriod(int millseconds){
    QTime t = QTime::currentTime();
    QTime s;
    s = t.addMSecs(millseconds);
    while(s!=QTime::currentTime()){
        QCoreApplication::processEvents();
    }
}

void ToplogyScene::commomOperation(){
    if(buttonclickedvents.size()==2){
        qDebug()<<"Stack size:  "<<buttonclickedvents.size();
        auto r = buttonclickedvents.pop();//出栈,注意出栈后，栈仍然保持，因此额外做一次清除操作
        auto r1 =buttonclickedvents.pop();//出栈,注意出栈后，栈仍然保持，因此额外做一次清除操作
        emit shutdownTrainsignal(r); //按钮X-A1灭灯,在灭灯之后
        emit shutdownTrainsignal(r1); //按钮X-A1灭灯,在灭灯之后
    }
    emit shutdownTrainsignal(the_last_clicked_button_name);
}

//在这个地方只做正反位判断，后面要加入锁定道岔的交互提示,所谓发撒子码是和列车所在位置紧密相关的
//we only arrage route if conditons are satisfied, however, we update all signal status in case of segementation status change

//finally, we emit maxu signal to trains

void ToplogyScene::tryinterlockingAroute(QString starx, QString endx){

    if(starx=="" || endx==""){
        return;
    }
    qDebug()<<"进入连锁逻辑区域..........."<<starx<<"...."<<endx;
    //在成功地申请到一条进路后，保留该进路的同时，开始驱动
    //下达各个区段的发码命令，每次的逻辑要understand
    //需要每个列车所在的位置，并且要记得发码
    //第一个逻辑步骤是，如果该线路上还有列车，则将闪烁的两个对应按钮恢复成默认的不闪烁状态，
    //第二个逻辑步骤是，驱动和观察指定的道岔等是否已经在对应的位置
    //有一个线程不断采集所有轨接继电器状态
    //与此同时，把界面的按钮驱动到原始状态，然后调用buttonclickedvents.clear();把按钮单击事件栈清空
    if("X"==starx){ //正向接车进路
        if(anchortracktoplogy->reseveredPath.contains(starx+">"+endx)){  //表示这条进路被占用的，因此要进行提示
            return;
        }
        //第二步：检查轨道继电器状态
        qDebug()<<"....检查轨道继电器状态 ...................";
        if("SI"==endx || "S3"==endx || "SII"==endx || "S4"==endx){

            auto gjzt = anchortracktoplogy->getSegmentStatus("3-11G");
            if(gjzt==1 || gjzt==-1){
                if(1==gjzt){
                    qDebug()<<"3-11G轨道区段已被占用 ...................";
                }else{
                    qDebug()<<"未检查到3-11G轨道区段状态 ...................";
                }
                commomOperation();
                return;
            }
            if("SI"==endx)
            {
                qDebug()<<"检查1G状态 ...................";
                gjzt = anchortracktoplogy->getSegmentStatus("1G");
                if(gjzt==1 || gjzt==-1){
                    //提示后恢复进路选择状态,也就说要让闪烁的按钮重新置
                    commomOperation();
                    return;
                }
                qDebug()<<"检查敌对信号机状态 ...................";
                int temp_signal_status  =anchortracktoplogy->getSingalStatus("S1");
                //当信号机颜色是3时，表示是红灯或者灭灯的时候可以开放进路
                if(temp_signal_status==3){
                    qDebug()<<"S1敌对信号机状态 ...................=3";
                    emit allowRouteSignal(starx,endx);    //将其关联到对应处理函数
                }else{
                    qDebug()<<"S1敌对信号机状态 ...................等于"<<temp_signal_status;
                    commomOperation();
                    return;
                }
            }else if("S3"==endx){
                qDebug()<<"检查3G状态......";
                auto gjzt_temp = anchortracktoplogy->getSegmentStatus("3G");
                if(gjzt_temp==1 || gjzt_temp==-1){ //该轨道已经被占用
                    commomOperation();
                    return;
                }
                int switchArrar[] = {1,3,5,7};
                for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
                    QString tempkey = QString::number(switchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==-1 || swzt==1){
                        // emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],0));
                    }
                }
                int fwswitchArrar[] = {11};
                for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
                    QString tempkey = QString::number(fwswitchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==0 || swzt==-1){
                        // emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],1));
                    }
                }
                if(anchortracktoplogy->getSingalStatus(endx)==3){   //提示信号灯故障错误
                    emit allowRouteSignal(starx,endx);
                    return;
                }
                commomOperation();
                return;

            }else if("SII"==endx){

                gjzt = anchortracktoplogy->getSegmentStatus("1-9G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }
                gjzt = anchortracktoplogy->getSegmentStatus("IIG");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }

                int switchArrar[] = {1,3,9};   //1，3，9定位
                for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
                    QString tempkey = QString::number(switchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==-1 || swzt==1){
                        //  emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],0));
                    }
                }
                int fwswitchArrar[] = {5,7};
                for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
                    QString tempkey = QString::number(fwswitchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==0 || swzt==-1){
                        //  emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],1));
                    }
                }
                //获取信号机状态
                qDebug()<<"....获取信号机"<<endx<<"的状态.....";
                auto counter_signal_name = endx;
                int temp_signal_status  =anchortracktoplogy->getSingalStatus(counter_signal_name);
                qDebug()<<"SII信号机状态....."<<temp_signal_status;
                if(anchortracktoplogy->getSingalStatus(counter_signal_name)==3){
                    emit allowRouteSignal(starx,endx);
                    return;
                }
                commomOperation();
                return;
            }else if("S4"==endx){
                qDebug()<<"进入S4这个地方..............................";
                gjzt = anchortracktoplogy->getSegmentStatus("1-9G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }
                gjzt = anchortracktoplogy->getSegmentStatus("4G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }
                int switchArrar[] = {1,3,9};
                for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
                    QString tempkey = QString::number(switchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==1 || swzt==-1){
                    }
                }
                int fwswitchArrar[] = {5,7};
                for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
                    QString tempkey = QString::number(fwswitchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==0 || swzt==-1){
                    }
                }

                auto counter_signal_name = endx;
                int temp_signal_status  =anchortracktoplogy->getSingalStatus(counter_signal_name);
                qDebug()<<"S4信号机状态....."<<temp_signal_status;
                if(anchortracktoplogy->getSingalStatus(counter_signal_name)==3){
                    emit allowRouteSignal(starx,endx);
                    return;
                }
                commomOperation();
                return;
            }
            else{
                return;
            }
        }

    }//进路
    else if("XN"==starx){


        //emit signalShouldDisplayStatus("XN-B2","green");

        if(anchortracktoplogy->reseveredPath.contains(starx+">"+endx)){  //表示这条进路被占用的，因此要进行提示
            return;
        }
        if(endx=="SII" ||endx=="S4" ||endx=="SI" ||endx=="S3" ){
            int gjzt;
            if(endx=="SII"){
                auto gjzt = anchortracktoplogy->getSegmentStatus("1-9G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }
                gjzt = anchortracktoplogy->getSegmentStatus("IIG");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }
                int switchArrar[] = {1,3,5,7,9};
                for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
                    QString tempkey = QString::number(switchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==1 || swzt==-1){
                        //  emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],0));
                    }
                }
                auto counter_signal_name = endx;
                int temp_signal_status  =anchortracktoplogy->getSingalStatus(counter_signal_name);
                qDebug()<<counter_signal_name<<"信号机状态....."<<temp_signal_status;
                if(anchortracktoplogy->getSingalStatus(counter_signal_name)==3){
                    emit allowRouteSignal(starx,endx);
                    return;
                }
                commomOperation();
                return;
            }
            else if(endx=="S4"){

                gjzt = anchortracktoplogy->getSegmentStatus("4G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }
                int switchArrar[] = {1,3,5,7};
                for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
                    QString tempkey = QString::number(switchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==1 || swzt==-1){
                        //   emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],0));
                    }
                }
                int fwswitchArrar[] = {9};
                for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
                    QString tempkey = QString::number(fwswitchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==0 || swzt==-1){
                        //emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],1));
                    }
                }
                auto counter_signal_name = endx;
                int temp_signal_status  =anchortracktoplogy->getSingalStatus(counter_signal_name);
                qDebug()<<counter_signal_name<<"信号机状态....."<<temp_signal_status;
                if(anchortracktoplogy->getSingalStatus(counter_signal_name)==3){
                    emit allowRouteSignal(starx,endx);
                    return;
                }
                commomOperation();
                return;
            }
            else if(endx=="SI"){

                gjzt = anchortracktoplogy->getSegmentStatus("3-11G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }
                gjzt = anchortracktoplogy->getSegmentStatus("1-9G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }

                gjzt = anchortracktoplogy->getSegmentStatus("1G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }

                int switchArrar[] = {1,3,5,7,11};
                for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
                    QString tempkey = QString::number(switchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==1 || swzt==-1){
                        //    emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],0));
                    }
                }
                auto counter_signal_name = endx;
                int temp_signal_status  =anchortracktoplogy->getSingalStatus("S1");
                qDebug()<<counter_signal_name<<"信号机状态....."<<temp_signal_status;
                if(anchortracktoplogy->getSingalStatus("S1")==3){
                    emit allowRouteSignal(starx,endx);
                    return;
                }
                commomOperation();
                return;

            }  else if(endx=="S3"){

                gjzt = anchortracktoplogy->getSegmentStatus("3-11G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }
                gjzt = anchortracktoplogy->getSegmentStatus("1-9G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }
                gjzt = anchortracktoplogy->getSegmentStatus("1G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }

                int switchArrar[] = {1,3,11};
                for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
                    QString tempkey = QString::number(switchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==0 || swzt==-1){
                        //   emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],1));
                    }
                }

                int fwswitchArrar[] = {5,7};
                for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
                    QString tempkey = QString::number(fwswitchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==1 || swzt==-1){
                        //emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],0));
                    }
                }
                auto counter_signal_name = endx;
                int temp_signal_status  =anchortracktoplogy->getSingalStatus(counter_signal_name);
                qDebug()<<counter_signal_name<<"信号机状态....."<<temp_signal_status;
                if(anchortracktoplogy->getSingalStatus(counter_signal_name)==3){
                    emit allowRouteSignal(starx,endx);
                    return;
                }
                commomOperation();
                return;

            } else{
                return;
            }
        }
    }else if(starx=="S"){

        if(endx=="XII" || endx=="X4"){

            auto gjzt = anchortracktoplogy->getSegmentStatus("2DG");
            if(gjzt==1 || gjzt==-1){
                commomOperation();
                return;
            }
            if(endx=="XII"){

                gjzt = anchortracktoplogy->getSegmentStatus("IIG");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }

                int switchArrar[] = {2};
                for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
                    QString tempkey = QString::number(switchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==1 || swzt==-1){
                        //   emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],0));
                    }
                }
                auto counter_signal_name = endx;
                int temp_signal_status  =anchortracktoplogy->getSingalStatus(counter_signal_name);
                qDebug()<<counter_signal_name<<"信号机状态....."<<temp_signal_status;
                if(anchortracktoplogy->getSingalStatus(counter_signal_name)==3){
                    qDebug()<<"运行到进路状态......";
                    emit allowRouteSignal(starx,endx);
                    return;
                }
                commomOperation();
                return;

            }else if(endx=="X4"){

                gjzt = anchortracktoplogy->getSegmentStatus("4G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }
                int switchArrar[] = {2};
                for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
                    QString tempkey = QString::number(switchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==1 || swzt==-1){
                        //    emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],0));
                    }
                }
                auto signal_name= endx;
                if(anchortracktoplogy->getSingalStatus(signal_name)==3){   //提示信号灯故障错误
                    emit allowRouteSignal(starx,endx);
                    return;
                }
                commomOperation();
                return;
            }else{
                return;
            }

        }

    } else if(starx=="SN"){

        if(endx=="XI" || endx=="X3"){

            auto gjzt = anchortracktoplogy->getSegmentStatus("4DG");
            if(gjzt==1 || gjzt==-1){
                commomOperation();
                return;
            }
            if(endx=="XI"){

                gjzt = anchortracktoplogy->getSegmentStatus("1G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }

                int switchArrar[] = {4};
                for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
                    QString tempkey = QString::number(switchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==1 || swzt==-1){
                        //   emit emitDriverCommand(wrapComandforswitchOperation(switchArrar[i],0));
                    }
                }
                auto signal_name= endx;
                if(anchortracktoplogy->getSingalStatus(signal_name)==3){   //提示信号灯故障错误
                    emit allowRouteSignal(starx,endx);
                    return;
                }
                commomOperation();
                return;
            }else if(endx=="X3"){
                gjzt = anchortracktoplogy->getSegmentStatus("3G");
                if(gjzt==1 || gjzt==-1){
                    commomOperation();
                    return;
                }
                int fwswitchArrar[] = {4};
                for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
                    QString tempkey = QString::number(fwswitchArrar[i]);
                    auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
                    if(swzt==0 || swzt==-1){
                        //   emit emitDriverCommand(wrapComandforswitchOperation(fwswitchArrar[i],1));
                    }
                }
                auto signal_name= endx;
                if(anchortracktoplogy->getSingalStatus(signal_name)==3){   //提示信号灯故障错误
                    emit allowRouteSignal(starx,endx);
                    return;
                }
                commomOperation();
                return;
            }else{
                return;
            }
        }

    } else if(starx=="XI" && endx=="SN"){

        auto gjzt = anchortracktoplogy->getSegmentStatus("4DG");
        if(gjzt==1 || gjzt==-1){
            commomOperation();
            return;
        }
        gjzt = anchortracktoplogy->getSegmentStatus("X1LQG");
        if(gjzt==1 || gjzt==-1){
            commomOperation();
            return;
        }
        int fwswitchArrar[] = {4};
        for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
            QString tempkey = QString::number(fwswitchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==0 || swzt==-1){
                //  emit emitDriverCommand(wrapComandforswitchOperation(fwswitchArrar[i],1));
            }
        }
        auto signal_name= endx;
        if(anchortracktoplogy->getSingalStatus(signal_name)!=3 || anchortracktoplogy->getSingalStatus(signal_name)!=0 ){   //提示信号灯故障错误
            return;
        }
        emit allowRouteSignal(starx,endx);
        return;
    }else if(starx=="X3" && endx=="SN"){
        auto gjzt = anchortracktoplogy->getSegmentStatus("4DG");
        if(gjzt==1 || gjzt==-1){
            return;
        }
        gjzt = anchortracktoplogy->getSegmentStatus("X1LQG");
        if(gjzt==1 || gjzt==-1){
            return;
        }
        int fwswitchArrar[] = {4};
        for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
            QString tempkey = QString::number(fwswitchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==0 || swzt==-1){
                return;
            }
        }
        auto signal_name= endx;
        if(anchortracktoplogy->getSingalStatus(signal_name)!=3 || anchortracktoplogy->getSingalStatus(signal_name)!=0 ){   //提示信号灯故障错误
            return;
        }
        emit allowRouteSignal(starx,endx);
        return;
    }
    else if(starx=="XII" && endx=="S"){
        auto gjzt = anchortracktoplogy->getSegmentStatus("2DG");
        if(gjzt==1 || gjzt==-1){
            return;
        }
        int switchArrar[] = {2};
        for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
            QString tempkey = QString::number(switchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==1 || swzt==-1){
                return;
            }
        }
        auto signal_name= endx;
        if(anchortracktoplogy->getSingalStatus(signal_name)!=3 || anchortracktoplogy->getSingalStatus(signal_name)!=0 ){   //提示信号灯故障错误
            return;
        }
        emit allowRouteSignal(starx,endx);
        return;
    }
    else if(starx=="X4" && endx=="S"){
        auto gjzt = anchortracktoplogy->getSegmentStatus("2DG");
        if(gjzt==1 || gjzt==-1){
            return;
        }
        int switchArrar[] = {2};
        for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
            QString tempkey = QString::number(switchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==0 || swzt==-1){
                return;
            }
        }
        auto signal_name= endx;
        if(anchortracktoplogy->getSingalStatus(signal_name)!=3 || anchortracktoplogy->getSingalStatus(signal_name)!=0 ){   //提示信号灯故障错误
            return;
        }
        emit allowRouteSignal(starx,endx);
        return;
    }
    else if(starx=="SII" && endx=="XN"){

        auto gjzt = anchortracktoplogy->getSegmentStatus("1-9DG");
        if(gjzt==1 || gjzt==-1){
            return;
        }
        gjzt = anchortracktoplogy->getSegmentStatus("S1LQG");
        if(gjzt==1 || gjzt==-1){
            return;
        }
        int switchArrar[] = {1,3,5,7,9};
        for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
            QString tempkey = QString::number(switchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==1 || swzt==-1){
                return;
            }
        }
        auto signal_name= endx;
        if(anchortracktoplogy->getSingalStatus(signal_name)!=3 || anchortracktoplogy->getSingalStatus(signal_name)!=0 ){   //提示信号灯故障错误
            return;
        }
        emit allowRouteSignal(starx,endx);
        return;
    }

    else if(starx=="S4" && endx=="XN"){

        auto gjzt = anchortracktoplogy->getSegmentStatus("1-9DG");
        if(gjzt==1 || gjzt==-1){
            return;
        }
        gjzt = anchortracktoplogy->getSegmentStatus("S1LQG");
        if(gjzt==1 || gjzt==-1){
            return;
        }

        int switchArrar[] = {1,3,5,7};
        for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
            QString tempkey = QString::number(switchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==1 || swzt==-1){
                return;
            }
        }

        int fwswitchArrar[] = {9};
        for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
            QString tempkey = QString::number(fwswitchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==0 || swzt==-1){
                return;
            }
        }

        auto signal_name= endx;
        if(anchortracktoplogy->getSingalStatus(signal_name)!=3 || anchortracktoplogy->getSingalStatus(signal_name)!=0 ){   //提示信号灯故障错误
            return;
        }
        emit allowRouteSignal(starx,endx);
        return;
    }

    else if(starx=="SI" && endx=="XN"){

        auto gjzt = anchortracktoplogy->getSegmentStatus("1-9DG");
        if(gjzt==1 || gjzt==-1){
            return;
        }


        gjzt = anchortracktoplogy->getSegmentStatus("3-11G");
        if(gjzt==1 || gjzt==-1){
            return;
        }


        gjzt = anchortracktoplogy->getSegmentStatus("S1LQG");
        if(gjzt==1 || gjzt==-1){
            return;
        }



        int switchArrar[] = {5,7,11};
        for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
            QString tempkey = QString::number(switchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==1 || swzt==-1){
                return;
            }
        }

        int fwswitchArrar[] = {1,3};
        for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
            QString tempkey = QString::number(fwswitchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==0 || swzt==-1){
                return;
            }
        }

        auto signal_name= endx;
        if(anchortracktoplogy->getSingalStatus(signal_name)!=3 || anchortracktoplogy->getSingalStatus(signal_name)!=0 ){   //提示信号灯故障错误
            return;
        }
        emit allowRouteSignal(starx,endx);
        return;
    }


    else if(starx=="S3" && endx=="XN"){

        auto gjzt = anchortracktoplogy->getSegmentStatus("1-9DG");
        if(gjzt==1 || gjzt==-1){
            return;
        }


        gjzt = anchortracktoplogy->getSegmentStatus("3-11G");
        if(gjzt==1 || gjzt==-1){
            return;
        }


        gjzt = anchortracktoplogy->getSegmentStatus("S1LQG");
        if(gjzt==1 || gjzt==-1){
            return;
        }



        int switchArrar[] = {5,7};
        for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
            QString tempkey = QString::number(switchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==1 || swzt==-1){
                return;
            }
        }

        int fwswitchArrar[] = {1,3,11};
        for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
            QString tempkey = QString::number(fwswitchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==0 || swzt==-1){
                return;
            }
        }

        auto signal_name= endx;
        if(anchortracktoplogy->getSingalStatus(signal_name)!=3 || anchortracktoplogy->getSingalStatus(signal_name)!=0 ){   //提示信号灯故障错误
            return;
        }
        emit allowRouteSignal(starx,endx);
        return;
    }


    else if(starx=="SI" && endx=="X"){

        auto gjzt = anchortracktoplogy->getSegmentStatus("3-11G");
        if(gjzt==1 || gjzt==-1){
            return;
        }


        gjzt = anchortracktoplogy->getSegmentStatus("1107G");
        if(gjzt==1 || gjzt==-1){
            return;
        }



        int switchArrar[] = {1,3,5,7,11};
        for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
            QString tempkey = QString::number(switchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==1 || swzt==-1){
                return;
            }
        }
        auto signal_name= endx;
        if(anchortracktoplogy->getSingalStatus(signal_name)!=3 || anchortracktoplogy->getSingalStatus(signal_name)!=0 ){   //提示信号灯故障错误
            return;
        }
        emit allowRouteSignal(starx,endx);
        return;
    }

    else if(starx=="S3" && endx=="X"){

        auto gjzt = anchortracktoplogy->getSegmentStatus("3-11G");
        if(gjzt==1 || gjzt==-1){
            return;
        }


        gjzt = anchortracktoplogy->getSegmentStatus("1107G");
        if(gjzt==1 || gjzt==-1){
            return;
        }



        int switchArrar[] = {1,3,5,7};
        for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
            QString tempkey = QString::number(switchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==1 || swzt==-1){
                return;
            }
        }

        int fwswitchArrar[] = {11};
        for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
            QString tempkey = QString::number(fwswitchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==0 || swzt==-1){
                return;
            }
        }

        auto signal_name= endx;
        if(anchortracktoplogy->getSingalStatus(signal_name)!=3 || anchortracktoplogy->getSingalStatus(signal_name)!=0 ){   //提示信号灯故障错误
            return;
        }
        emit allowRouteSignal(starx,endx);
        return;
    }
    else if(starx=="SII" && endx=="X"){

        auto gjzt = anchortracktoplogy->getSegmentStatus("3-11G");
        if(gjzt==1 || gjzt==-1){
            return;
        }


        gjzt = anchortracktoplogy->getSegmentStatus("1107G");
        if(gjzt==1 || gjzt==-1){
            return;
        }

        gjzt = anchortracktoplogy->getSegmentStatus("1-9DG");
        if(gjzt==1 || gjzt==-1){
            return;
        }


        int switchArrar[] = {1,3,9};
        for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
            QString tempkey = QString::number(switchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==1 || swzt==-1){
                return;
            }
        }

        int fwswitchArrar[] = {5,7};
        for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
            QString tempkey = QString::number(fwswitchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==0 || swzt==-1){
                return;
            }
        }
        auto signal_name= endx;
        if(anchortracktoplogy->getSingalStatus(signal_name)!=3 || anchortracktoplogy->getSingalStatus(signal_name)!=0 ){   //提示信号灯故障错误
            return;
        }
        emit allowRouteSignal(starx,endx);
        return;
    }

    else if(starx=="S4" && endx=="X"){

        auto gjzt = anchortracktoplogy->getSegmentStatus("3-11G");
        if(gjzt==1 || gjzt==-1){
            return;
        }


        gjzt = anchortracktoplogy->getSegmentStatus("1107G");
        if(gjzt==1 || gjzt==-1){
            return;
        }

        gjzt = anchortracktoplogy->getSegmentStatus("1-9DG");
        if(gjzt==1 || gjzt==-1){
            return;
        }


        int switchArrar[] = {1,3};
        for(int i=0;i<sizeof(switchArrar) /sizeof(switchArrar[0]);i++){
            QString tempkey = QString::number(switchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==1 || swzt==-1){
                return;
            }
        }

        int fwswitchArrar[] = {5,7,9};
        for(int i=0;i<sizeof(fwswitchArrar) /sizeof(fwswitchArrar[0]);i++){
            QString tempkey = QString::number(fwswitchArrar[i]);
            auto swzt = anchortracktoplogy->getSwitchStatus(tempkey);
            if(swzt==0 || swzt==-1){
                return;
            }
        }

        auto signal_name= endx;
        if(anchortracktoplogy->getSingalStatus(signal_name)!=3 || anchortracktoplogy->getSingalStatus(signal_name)!=0 ){   //提示信号灯故障错误
            return;
        }
        emit allowRouteSignal(starx,endx);
        return;
    }
    else{

        return;
    }

}

void ToplogyScene::driverSignalUpdate(QString signal_name, QString signal_color){
    this->signal2Color[signal_name]=signal_color;
}


void ToplogyScene::timeoutjob(){
    if(buttonclickedvents.size()<2 && buttonclickedvents.size()>0){
        auto topbutton_name= buttonclickedvents.pop();
        //发出一个取消选择信号，也就是让按钮停止闪烁，停止闪烁也就是重新设置进路起始点为0
        qDebug()<<"time out job entering....";
        emit buttonTargetcalcelselect(topbutton_name);  //发送按钮停止选择信号
    }
    emit shutdownTrainsignal(the_last_clicked_button_name);
}


void ToplogyScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);

    if(QApplication::mouseButtons() == Qt::LeftButton){
        m_pItemSelected = nullptr;
        foreach(QGraphicsItem *item, items(event->scenePos()))//循环查找当前鼠标位置点下所有的Item
        {
            if(item->type() == QGraphicsProxyWidget::Type)//判断此Item是否为添加的ActorWidget创建的QGraphicsProxyWidget
            {
                m_bPressed = true;                         //设置鼠标被按下了，在特定区域中
                QGraphicsProxyWidget *pProxyWidget = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
                QPointF point = pProxyWidget->mapToScene(QPointF(0.0, 0.0));  //图元左上角坐标
                m_shiftOrg.setX(event->scenePos().x() - point.x());
                m_shiftOrg.setY(event->scenePos().y() - point.y());
                m_pItemSelected = item;
                m_ItemPos = pProxyWidget->scenePos(); //QGraphicsProxyWidget的位置点,我们希望通过场景位置点来区别各个代理窗口位置
                QString x,y;
                x.setNum(m_ItemPos.rx());       //应该以这个位置为主
                y.setNum(m_ItemPos.ry());
                qDebug()<<"...."<<x<<"....."<<y;
                QString item_name="";
                QMap<QString,QString>::Iterator it=anchortracktoplogy->item_location_map.begin(); //把位置变化为名称
                while(it!=anchortracktoplogy->item_location_map.end())
                {
                    QStringList str_list = it.value().split("|");
                    if(str_list[0]==x && str_list[1]==y){
                        item_name =it.key();
                    }
                    it++;
                }
                //将item_name作为参数发送到相关事件. https://blog.csdn.net/u010058695/article/details/100153249 (将QT项目打包的过程)
                if(item_name.length()>0){
                    //在这里就开始判断是否是引导进路按钮被点击
                    //引导进路按键
                    the_last_clicked_button_name = item_name;
                    if(item_name=="X-A2"){
                        emit buttonTargetName(item_name);  //首先让其进行闪烁，然后
                        break;
                    }
                    if(buttonclickedvents.size()==2 ){   //不允许多次点击相同按钮
                        if(this->anchortracktoplogy->fromStation
                                ==item_name.split("|")[0]){
                            break;
                        }
                        if( this->anchortracktoplogy->toStation==item_name.split("|")[0]){
                            break;
                        }


                    }else if (buttonclickedvents.size()==1){
                        if(this->anchortracktoplogy->fromStation==item_name.split("-")[0]){
                            break;
                        }
                    }else{

                    }
                    buttonclickedvents.push(item_name);
                    emit buttonTargetName(item_name);  //首先让其进行闪烁，然后
                    qDebug()<<"场景中"<<item_name<<"按钮被按下.......";
                    QTimer::singleShot(10000, this, SLOT(timeoutjob()) );
                }
                break;
            }
        }

    }
}


//我们在优先级最高的event中处理联锁事件类型
bool ToplogyScene::event(QEvent *event)
{
    QEvent::Type eventType = event->type();

    if((eventType == Interlockingevent::eventType))
    {
        /*在这里处理事件*/
        Interlockingevent *myEvent = dynamic_cast<Interlockingevent*>(event);
        qDebug() << myEvent->getValue1();
        qDebug() << myEvent->getValue2();
        qDebug()<<"I have gotten the custome event in the ToplogyScene......";
        //我们将该事件发送到graphicitem
        QList<QGraphicsItem*> itemList = this->items();
        for (int i = 0; i < itemList.size(); i++)
        {
            qDebug() << itemList.at(i)->type() << itemList.at(i)->pos();
        }
        //什么叫转换？？？，这个地方可以依据类型来判断,这些QGraphicsObject在屏幕上的位置必须记录下来，才能准确发送
        QGraphicsItem * mytraingitem = this->itemAt(-768,-242,dynamic_cast<QGraphicsView*>(this->parent())->transform());
        sendEvent(mytraingitem,event);
        return true;  //表示该自定义事件已经被处理
    }
    return QGraphicsScene::event(event);
}

void ToplogyScene::setToplogyBuiler(std::shared_ptr<TracktologyBuilder> &anchortracktoplogy){
    this->anchortracktoplogy = anchortracktoplogy;
}
