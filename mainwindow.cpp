#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QPropertyAnimation>
#include <QGraphicsScene>
#include <QGraphicsAnchorLayout>
#include <QGraphicsObject>
#include <QScreen>
#include <QTimer>
#include "Triangle.h"
#include "InterFilter.h"
#include "Interlockingevent.h"
#include "Myellipseitem.h"
#include "Mysignal.h"
#include <QPushButton>
#include "Tracktologybuilder.h"
#include <QGraphicsItemGroup>
#include "CustomGraphicsArrow.h"
#include <QPointF>
#include "MyRectItem.h"
#include "MyLineItem.h"
#include <QSerialPort>
#include <QStringList>
#include <QTextStream>
#include <QSerialPortInfo>
#include <QStackedWidget>
#include "segmentcircuitdiagram.h"
#include "MenuPushButton.h"
#include <QDialog>
#include <QVBoxLayout>
#include "CustomDialog.h"
#include "ReceiverThread.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mStackedWidget(new QStackedWidget(this))
    ,msegmentcidigmwidget(new SegmentCircuitDiagram(this))
{
    //注意下面的调用次序 420115201310254727
    initialScene();                 //初始化场景有关参数
    setSceneToplogyStore();         //为场景设置topbuilder以存储场景中的拓扑结构
    configureOutterTrack();         //配置外圈
    configureinnerTrack();          //配置里圈和信号灯灯
    addHiddenRouterEntrenceButton();
    configurXiaweijicomunication();
    addFunctionButtons(scene_size); //增加下行功能区
    addFourGroupsForIndication();   //增加四个指示灯区
    placeMarker();                  //增加一些标志,因为都是不动的，所以可以这么弄
    addCriticalIsolations();        //在拓扑上安装绝缘结
    manageStackedWidgets();         //管理界面切换,就用QML算了，不想那么多了就用一个全屏的界面搞
    initalallDeviceSingalStatus();  //在此函数中关联信号更新操作,先把铁路拓扑中的道岔、转辙机，以及信号机的所有信息都更新一遍
    associateTopscnewithMainwindows();
    bindRouteRequesttolockinglogic();
    //重写ToplogyScene的mousePressEvent事件函数，当在界面上选择按钮时候，触发进路操作
    //在Mainwindow的addButtonToScene函数中，增加了对按钮触发时候的整个窗口逻辑
    //在ToplogyScene::requestRouteHandler触发请求进路处理
    //在TracktologyBuilder中触发addRouteRequest
    connect(ui->comboBox,&QComboBox::currentTextChanged,this,&MainWindow::comboxItemchange);
    initalInterlockingSceneStatus();
}



//根据罗老师图设置状态
void MainWindow::initalInterlockingSceneStatus(){
    updateswithoperationsinthescene(QString::number(1),"directed");
    updateswithoperationsinthescene(QString::number(2),"directed");
    updateswithoperationsinthescene(QString::number(3),"directed");
    updateswithoperationsinthescene(QString::number(4),"directed");
    updateswithoperationsinthescene(QString::number(5),"directed");
    updateswithoperationsinthescene(QString::number(7),"directed");
    updateswithoperationsinthescene(QString::number(9),"directed");
    updateswithoperationsinthescene(QString::number(11),"directed");
}

void MainWindow::addHiddenRouterEntrenceButton(){  //这里要自己定义一个对话框,表示引导进路开始

    //---------------------------------------------------------------
    string*  name = new string("");  //堆内存分配和栈内存分配不一样
    dialog_x_gaifang = new CustomDialog("X改方-123",nullptr);
    dialog_x_gaifang->setGeometry(ax_startx-50,ay_startx-180,20,20);
    dialog_x_gaifang->setWindowOpacity(0.0);
    dialog_x_gaifang->addLabel("X改方-123");  //初始为透明
    dialog_x_gaifang->addLineEdit ("", name, "123");
    mtoplogyscene->addWidget(dialog_x_gaifang);
}


//每个发码都是发送全部，假定输入是"L|L|L|L|L|L|L|L",假定8个区段都要发送
bool MainWindow::communicationFaMa(QString fama){

    //X1LQG-1101G-1103G-1105G-1107G-3-11G-1G-4DG
    //"JC|HU|JC|LU|L|LU|U|HU"
    //X1LQG|3-11G|1101G|1G|1103G|4DG|1105G|1107G  （V5.1）
    QByteArray ba;
    ba.resize(5);
    ba[0]=0xfa;
    int j = 1;
    auto zz = fama.split("|");
    qDebug()<<"zz。。。。。。。。。。。。。。。。。。。。"<<zz;
    char tempbytearr[8];
    for(int i=0;i<zz.length();i++){
        if(zz[i]=="L"){
            tempbytearr[i]=0x00;
        }else if(zz[i]=="LU"){
            tempbytearr[i]=0x01;
        }
        else if(zz[i]=="U"){
            tempbytearr[i]=0x02;
        }
        else if(zz[i]=="UU"){
            tempbytearr[i]=0x03;
        }
        else if(zz[i]=="U2S"){
            tempbytearr[i]=0x04;

        }else if(zz[i]=="U2"){
            tempbytearr[i]=0x05;
        }
        else if(zz[i]=="UUS"){
            tempbytearr[i]=0x06;
        }
        else if(zz[i]=="HB"){
            tempbytearr[i]=0x07;
        }
        else if(zz[i]=="HU"){
            tempbytearr[i]=0x08;
        }
        else if(zz[i]=="LU2"){
            tempbytearr[i]=0x09;
        }

        else if(zz[i]=="ZQ"){
            tempbytearr[i]=0x0a;
        }
        else if(zz[i]=="JC"){
            tempbytearr[i]=0x0b;
        }
        else if(zz[i]=="H"){
            tempbytearr[i]=0x0c;
        }
        else if(zz[i]=="L2"){
            tempbytearr[i]=0x0d;
        }
        else if(zz[i]=="L3"){
            tempbytearr[i]=0x0e;
        }else{
            continue;
        }
    }
    ba[1] =(char)(((tempbytearr[0]<<4)& 0xff)| tempbytearr[1]);
    ba[2] = (char) (((tempbytearr[2]<<4)& 0xff)| tempbytearr[3]);
    ba[3] = (char) (((tempbytearr[4]<<4)& 0xff)| tempbytearr[5]);
    ba[4] = (char) (((tempbytearr[6]<<4)& 0xff)| tempbytearr[7]);
    // qDebug()<<"16进制小写输出ba[1]..."<<ba[1];

    uint16_t wcrc = Helper::ModbusCRC16(ba);
    unsigned char crcdi=uint8_t(wcrc);
    unsigned char crcgao=uint8_t(wcrc>>8);
    qDebug()<<"wcrc=....."<<wcrc;
    QByteArray re;
    re.resize(7);
    re[0]=ba[0];
    re[1]=ba[1];
    re[2]=ba[2];
    re[3]=ba[3];
    re[4]=ba[4];
    re[5]=crcdi;
    re[6]=crcgao;
    qDebug()<<"开始发送发码指令。。。。。。。。。。。。。。。。。。。。。。。。"<<re.size();
    qDebug()<<"开始发送发码指令。。。。。。。。。。。。。。。。。。。。。。。。。。。"<<re.toHex();
    this->m_thread_xiaweiji->sendMessageToSeriealPort(re);
    return true;
}


void MainWindow::comboxItemchange(const QString &text){
    qDebug()<<"In MainWindow ................."<<text;
    qDebug() << "........... serialPortName..." <<serialPortName;
    configurXiaweijicomunication(); //配置与下位机通信模块
}

//初始化连锁逻辑中需要关联的事件和槽,这个地方是最重要的
void MainWindow::bindRouteRequesttolockinglogic(){
    connect(this->tracktopstore, &TracktologyBuilder::addRouteRequest,this->mtoplogyscene,&ToplogyScene::tryinterlockingAroute);
}


void MainWindow::updateReserverPathinthescene(QString from, QString to, QString color){
    //得到from的位置
    qDebug()<<"in MainWindow::updateReserverPathinthescene.....from"<<from<<".....to"<<to;

    QMap<QString, QPointF>::iterator iter = this->tracktopstore->criticalPointsinToplogy.begin();
    while (iter != this->tracktopstore->criticalPointsinToplogy.end())
    {
        //qDebug() << "Iterator " << iter.key() << ":" << iter.value(); // 迭代器
        iter++;
    }
    if(this->tracktopstore->criticalPointsinToplogy.contains(from)==false
            || this->tracktopstore->criticalPointsinToplogy.contains(to)==false){
        return;
    }
    //qDebug()<<"ought run here in MainWindow::updateReserverPathinthescene.....";
    auto fromscenepoint =this->tracktopstore->criticalPointsinToplogy.value(from);
    auto toscenepoint =this->tracktopstore->criticalPointsinToplogy.value(to);

    QLineF line(fromscenepoint,toscenepoint);
    MyLineItem *mylineline = new MyLineItem(line);  //默认的线段宽度是3个像素
    mtoplogyscene->update(mylineline->boundingRect());
    mtoplogyscene->addItem(mylineline);
    if(color=="white"){
        QColor t =Qt::white;
        mylineline->setColor(t);
    }else if(color=="red"){
        QColor t =Qt::red;
        mylineline->setColor(t);

    }else if(color=="black"){
        QColor t =Qt::black;
        mylineline->setColor(t);
    }
    else if(color=="green"){
        QColor t =Qt::green;
        mylineline->setColor(t);
    }
    else if(color=="yellow"){
        QColor t =Qt::yellow;
        mylineline->setColor(t);
    }
    else{          //恢复成原来的颜色
        QColor t;
        t.setRed(77);
        t.setGreen(118);
        t.setBlue(179);
        mylineline->setColor(t);
    }
    mtoplogyscene->update(mylineline->boundingRect());
}


void MainWindow::updateswithoperationsinthescene(QString whichswitch,QString whichoperation){

    int upordown_distance=7;
    QColor t = Qt::black;
    if(whichswitch=="1" && whichoperation=="directed"){  //1号道岔定向操作
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //定向操作就是1的上面一小段黑色的
        QPointF startF(poinf1.rx(),poinf1.ry()-upordown_distance);
        QPointF endF(poinf1.rx()+5,poinf1.ry()-upordown_distance);
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
         mtoplogyscene->update();
        return;
    }
    if(whichswitch=="1" && whichoperation=="reversed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //前面一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry());
        QPointF endF(poinf1.rx()+5,poinf1.ry());
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        return;
    }
    if(whichswitch=="2" && whichoperation=="directed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry()+upordown_distance);
        QPointF endF(poinf1.rx()-upordown_distance,poinf1.ry()+upordown_distance);
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        return;
    }

    if(whichswitch=="2" && whichoperation=="reversed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry());
        QPointF endF(poinf1.rx()-5,poinf1.ry());
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        return;
    }
    if(whichswitch=="3" && whichoperation=="directed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry()+5);
        QPointF endF(poinf1.rx()-5,poinf1.ry()+5);
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        mtoplogyscene->update();
        return;
    }

    if(whichswitch=="3" && whichoperation=="reversed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry());
        QPointF endF(poinf1.rx()-5,poinf1.ry());
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        return;
    }
    if(whichswitch=="4" && whichoperation=="directed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry()-5);
        QPointF endF(poinf1.rx()-5,poinf1.ry()-5);
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        mtoplogyscene->update();
        return;
    }

    if(whichswitch=="4" && whichoperation=="reversed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry());
        QPointF endF(poinf1.rx()-5,poinf1.ry());
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        return;
    }



    if(whichswitch=="5" && whichoperation=="directed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry()+upordown_distance);
        QPointF endF(poinf1.rx()+7,poinf1.ry()+upordown_distance);
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        mtoplogyscene->update();
        return;
    }

    if(whichswitch=="5" && whichoperation=="reversed"){

        QColor t = Qt::yellow;
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry());
        QPointF endF(poinf1.rx()+5,poinf1.ry());
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        return;
    }


    if(whichswitch=="7" && whichoperation=="directed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry()-upordown_distance);
        QPointF endF(poinf1.rx()-5,poinf1.ry()-upordown_distance);
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        mtoplogyscene->update();

        return;
    }

    if(whichswitch=="7" && whichoperation=="reversed"){

        QColor t = Qt::yellow;
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry());
        QPointF endF(poinf1.rx()+5,poinf1.ry());
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        return;
    }



    if(whichswitch=="9" && whichoperation=="directed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry()+upordown_distance);
        QPointF endF(poinf1.rx()+7,poinf1.ry()+upordown_distance);
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        mtoplogyscene->update();
        return;
    }

    if(whichswitch=="9" && whichoperation=="reversed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry());
        QPointF endF(poinf1.rx()-5,poinf1.ry());
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        return;
    }

    if(whichswitch=="11" && whichoperation=="directed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry()+5);
        QPointF endF(poinf1.rx()-5,poinf1.ry()+5);
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
           mtoplogyscene->update();
        return;
    }

    if(whichswitch=="11" && whichoperation=="reversed"){
        auto poinf1 = this->tracktopstore->criticalPointsinToplogy.value(whichswitch);
        addGreenLinetoSwitch(poinf1,mtoplogyscene,Qt::green);
        //下面左边一段变成黑色
        QPointF startF(poinf1.rx(),poinf1.ry());
        QPointF endF(poinf1.rx()-5,poinf1.ry());
        QLineF line1(startF,endF);
        MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
        Sline1->setColor(t);
        mtoplogyscene->addItem(Sline1);
        return;
    }

}
void MainWindow::associateTopscnewithMainwindows(){
    connect(this->mtoplogyscene,&ToplogyScene::drawlineSignal,this,&MainWindow::updateReserverPathinthescene);
    connect(this->mtoplogyscene,&ToplogyScene::swithoperationsignaldisplay,this,&MainWindow::updateswithoperationsinthescene);
    connect(this->mtoplogyscene,&ToplogyScene::emitDriverCommand,this,&MainWindow::sendDriverCommand);
    connect(this->mtoplogyscene,&ToplogyScene::emitmaxubiao,this,&MainWindow::sendmaxucommand);
}



void MainWindow::popUpDiaglog()
{
    qDebug()<<"........popUpDiaglog......................";
    dialog_x_gaifang->setWindowOpacity(1);
    dialog_x_gaifang->update();

}

void MainWindow::sendmaxucommand(QString maxu){
    communicationFaMa(maxu);
}

void MainWindow::sendDriverCommand(QByteArray ba){
    this->m_thread_xiaweiji->sendMessageToSeriealPort(ba);
}



//初始化信号机状态，包括实物和非实物信号机状态，将这些信号机状态直接发送给轨道拓扑信号进行管理
void MainWindow::initalallDeviceSingalStatus(){
    connect(&*m_thread_xiaweiji, &ReceiverThread::feishiwusignalstatusreceieved, this->tracktopstore, &TracktologyBuilder::updatekeepSignalStatus);
    connect(&*m_thread_xiaweiji, &ReceiverThread::feishiwuswitchstatusreceived, this->tracktopstore, &TracktologyBuilder::updatekeepSwitchStatus);
    connect(&*m_thread_xiaweiji, &ReceiverThread::guidaoquduancaijistatus, this->tracktopstore, &TracktologyBuilder::updatekeepSegmentstatus);
    connect(&*m_thread_xiaweiji, &ReceiverThread::shiwudaochaxinhaojistatus, this->tracktopstore, &TracktologyBuilder::updateshiwuxinhaojihedaochaStatus);
}


void MainWindow::manageStackedWidgets(){

    mStackedWidget->addWidget(ui->centralwidget);
    mStackedWidget->addWidget(msegmentcidigmwidget);
    displayMainwindow();
    setCentralWidget(mStackedWidget);
    //每当点设置故障按钮的时候，进行切换
    connect(this->ui->SzgzButton, &QPushButton::clicked, this, &MainWindow::showSegmentBlockCirculDiagram);
    connect(this->msegmentcidigmwidget, &SegmentCircuitDiagram::backtomain,this,&MainWindow::SegmentBlockCirculDiagramBack);
    //每当在故障设置页面点故障下发时，会发送sendMessageToseriesport信号，在主界面种进行处理
    connect(this->msegmentcidigmwidget,&SegmentCircuitDiagram::sendMessageToseriesport, &*m_thread_xiaweiji, &ReceiverThread::startSendMessage);
}



void MainWindow::displayMainwindow(){
    mStackedWidget->setCurrentWidget(ui->centralwidget);
}




void MainWindow::setSceneToplogyStore(){
    this->tracktopstore = new TracktologyBuilder();
    std::shared_ptr<TracktologyBuilder> spw1(tracktopstore);  //在这里要学会使用智能指针传递，否则会造成错误
    //在场景事件管理器mtoplogyscene
    mtoplogyscene->setToplogyBuiler(spw1);
}

void MainWindow::addCriticalIsolations(){
    addIsolationToSingal(QPointF{endX3.rx(),ay_xn-5},QPointF{endX3.rx(),ay_xn+5},c1); //在XII处加入绝缘节
    addIsolationToSingal(QPointF{endX3.rx(),ay_xn+(tracktoplodgyview->height()/7)-5},QPointF{endX3.rx(),ay_xn+(tracktoplodgyview->height()/7)+5},c1); //在X4处加入绝缘节
    addIsolationToSingal(QPointF{sxcg_single_s3.rx(),ay_startx+5},QPointF{sxcg_single_s3.rx(),ay_startx-5},c1);  //SI的坐标位置
    addIsolationToSingal(QPointF{sxcg_single_s3.rx(),ay_startx+(tracktoplodgyview->height()/7)+5},QPointF{sxcg_single_s3.rx(),ay_startx+(tracktoplodgyview->height()/7)-5},c1);  //SII的坐标位置
    addIsolationToSingal(QPointF{sxcg_single_s3.rx(),ay_startx+(tracktoplodgyview->height()/7)*2+5},QPointF{sxcg_single_s3.rx(),ay_startx+(tracktoplodgyview->height()/7)*2-5},c1);  //SII的坐标位置
}
//这里的位置需要人工调节
void MainWindow::placeMarker(){
    addTextTolocation("X",QPointF{ax_startx-5,ay_startx-10-single_button_height-20},QColor(255,255,0));
    addTextTolocation("XTA",QPointF{ax_startx-5-4*single_button_width+5,ay_startx-10-single_button_height-20},QColor(255,255,0));
    addTextTolocation("XN",QPointF{ax_xn-5,ay_xn+10+single_button_height},QColor(255,255,0));
    addTextTolocation("5",QPointF{ax_startx+dist_X_5*track_len_ratio-10,ay_startx},QColor(255,255,0));
    addTextTolocation("3",QPointF{ax_startx+dist_x_3*track_len_ratio-5,ay_startx},QColor(255,255,0));
    addTextTolocation("11",QPointF{ax_startx+dist_x_11*track_len_ratio-10,ay_startx},QColor(255,255,0));
    addTextTolocation("4",QPointF{endSN.rx()-200*track_len_ratio+100-10,ay_startx},QColor(255,255,0));
    addTextTolocation("SN",QPointF{endSN.rx()+120-10,ay_startx-30-single_button_height},QColor(255,255,0));
    addTextTolocation("1101",QPointF{endSN.rx()+SN_1101_len*track_len_ratio-15,ay_startx-30-single_button_height},QColor(255,255,0));
    addTextTolocation("1103",QPointF{endSN.rx()+(SN_1101_len+_1101_1103_len)*track_len_ratio-15,ay_startx-30-single_button_height},QColor(255,255,0));
    addTextTolocation("1105",QPointF{endSN.rx()+(SN_1101_len+_1101_1103_len+_1103_1105_len)*track_len_ratio-15,ay_startx-30-single_button_height},QColor(255,255,0));
    addTextTolocation("1107",QPointF{endSN.rx()+(SN_1101_len+_1101_1103_len+_1103_1105_len+_1105_1107_len)*track_len_ratio-15,ay_startx-30-single_button_height},QColor(255,255,0));
    addTextTolocation("XI",QPointF{endSI.rx()+120+single_button_width,ay_startx-30-single_button_height},QColor(255,255,0));
    addTextTolocation("SI",QPointF{sxcg_single_s3.rx()-10,ay_startx+10.0+single_button_height},QColor(255,255,0));
    addTextTolocation("S3",QPointF{sxcg_single_s3.rx()-10,ay_startx-(tracktoplodgyview->height()/7.0)+single_button_height+10},QColor(255,255,0));
    addTextTolocation("1",QPointF{ax_startx+dist_X_5*track_len_ratio-10,ay_xn},QColor(255,255,0));
    addTextTolocation("7",QPointF{ax_startx+dist_x_3*track_len_ratio-10,ay_xn},QColor(255,255,0));
    addTextTolocation("9",QPointF{ax_startx+dist_x_11*track_len_ratio-10,ay_xn},QColor(255,255,0));
    addTextTolocation("2",QPointF{endSN.rx()-200*track_len_ratio+100,ay_xn},QColor(255,255,0));
    addTextTolocation("SII",QPointF{sxcg_single_s3.rx()-10,ay_xn+10+single_button_height},QColor(255,255,0));
    addTextTolocation("S",QPointF{endSN.rx()+120-5,ay_xn+10+single_button_height},QColor(255,255,0));
    addTextTolocation("STA",QPointF{endSN.rx()+120-5+2*single_button_width+5,ay_xn+10+single_button_height},QColor(255,255,0));
    addTextTolocation("1102",QPointF{endSN.rx()+SN_1101_len*track_len_ratio-15,ay_xn+10+single_button_height},QColor(255,255,0));
    addTextTolocation("1104",QPointF{endSN.rx()+(SN_1101_len+_1101_1103_len)*track_len_ratio-15,ay_xn+10+single_button_height},QColor(255,255,0));
    addTextTolocation("1106",QPointF{endSN.rx()+(SN_1101_len+_1101_1103_len+_1103_1105_len)*track_len_ratio-15,ay_xn+10+single_button_height},QColor(255,255,0));
    addTextTolocation("1108",QPointF{endSN.rx()+(SN_1101_len+_1101_1103_len+_1103_1105_len+_1105_1107_len)*track_len_ratio-15,ay_xn+10+single_button_height},QColor(255,255,0));
    addTextTolocation("XII",QPointF{endSI.rx()+120+single_button_width,ay_startx-30-single_button_height+tracktoplodgyview->height()/7},QColor(255,255,0));
    addTextTolocation("X4",QPointF{endSI.rx()+120+single_button_width,ay_startx-30-single_button_height+(tracktoplodgyview->height()/7)*2},QColor(255,255,0));
    addTextTolocation("X3",QPointF{endSI.rx()+120+single_button_width,ay_startx-30-single_button_height-(tracktoplodgyview->height()/7)},QColor(255,255,0));
    //addTextTolocation("X3",QPointF{endSI.rx()+120+single_button_width,ay_startx-30-single_button_height-(tracktoplodgyview->height()/7)},QColor(255,255,0));
    addTextTolocation("S4",QPointF{sxcg_single_s3.rx()-10,ay_xn+10+single_button_height+tracktoplodgyview->height()/7},QColor(255,255,0));

}


void MainWindow::addFourGroupsForIndication(){
    qreal ax;
    qreal ay;
    ax = ax_startx-(this->pi)*(this->outterradiance)*0.2*track_len_ratio;
    //qDebug() << "........... ax....left group..." <<ax;
    ay = ay_startx-(tracktoplodgyview->height()/heightsequal)*2-50;
    //qDebug() << "........... ay....left group..." <<ay;
    CustomItemGroup* lefttopgroup;
    addGroupToscene(lefttopgroup,ax,ay,"X");
    CustomItemGroup* leftbottomgroup;
    qreal ax1=ax;
    qreal ay1=ay_startx+(tracktoplodgyview->height()/heightsequal)*2+50;
    // qDebug() << "........... ay1....left group..." <<ay1;
    addGroupToscene(leftbottomgroup,ax1,ay1,"XN");

    qreal ax2=endSN.rx()-200*track_len_ratio+100;
    qreal ay2=ay;
    CustomItemGroup* righttopgroup;
    addGroupToscene(righttopgroup,ax2,ay2,"SN");

    CustomItemGroup* rightbottomgroup;
    qreal ax3=endSN.rx()-200*track_len_ratio+100;
    qreal ay3=ay_startx+(tracktoplodgyview->height()/heightsequal)*2+50;
    addGroupToscene(rightbottomgroup,ax3,ay3,"S");

}


//关联combox的选择值变化事件和窗口通信机器
void MainWindow::configurXiaweijicomunication(){
    serialPortName=ui->comboBox->currentText();
    qDebug() << "........... serialPortName..." <<serialPortName;
    m_thread_xiaweiji = new ReceiverThread(this);
    connect(&*m_thread_xiaweiji, &ReceiverThread::shouldupdatexinhaojidaochazhuanzeji, this,&MainWindow::updatesceneandreal);  //得到更新的时候，说明要开始发码和改变信号灯颜色了
    connect(&*m_thread_xiaweiji, &ReceiverThread::refreshSementStausOfUI, this,&MainWindow::checkSegmentstatusAndRedrawLine);  //得到更新的时候，说明要开始发码和改变信号灯颜色了
    connect(&*m_thread_xiaweiji, &ReceiverThread::refreshSwitchStatusOfUI, this,&MainWindow::checkSwitchstatusAndRedrawRepresentation);  //得到更新的时候，说明要开始发码和改变信号灯颜色了
    m_thread_xiaweiji->startReceiver(this->serialPortName,50);
}


void MainWindow::checkSwitchstatusAndRedrawRepresentation(QString switchs){
    this->mtoplogyscene->checkSwitchsDirections(switchs);
}

//轨道区段变化信息，记住一个问题，当1107G没有被占用的时候，那个1107信号灯的颜色不应该是红色，这是个bug
void MainWindow::checkSegmentstatusAndRedrawLine(QString segments){
    //check segment and redraw line
    this->mtoplogyscene->checkSegmentTakenStatus(segments);
}


void MainWindow::configureinnerTrack(){
    QColor mycolor = Qt::black;
    //确定信号灯位置 第一个轨道上的信号灯-------------------------------------------------
    addIsolationToSingal(QPointF{ax_startx,ay_startx-10-single_button_height},QPointF{ax_startx,ay_startx-10},c1);
    addSingalToScene(x_signal_B1,ax_startx+5.0,ay_startx-10-single_button_height,mycolor,"X-B1");  //信号灯名称一定是需要的
    addSingalToScene(x_signal_B2,ax_startx+5.0+single_button_width,ay_startx-10-single_button_height,mycolor,"X-B2");
    int XN_SII_len = X_SI_len;  //这两个是相等的
    ax_xn = ax_startx;
    ay_xn = -(tracktoplodgyview->height()/2.0)+(tracktoplodgyview->height()/heightsequal)*4.0;
    QPointF startxn(ax_xn,ay_xn);
    QPointF endSII(ax_xn+(XN_SII_len)*track_len_ratio,ay_xn);
    MyLineItem *line_XN_SII = addLineToScene(startxn,endSII);

    this->tracktopstore->addCriticalPointsInformation("XN",startxn);  //增加

    //this->tracktopstore->addCriticalPointsInformation("SII",endSII);  //增加

    this->tracktopstore->addCriticalPointsInformation("SII",QPointF{endSII.rx()+80,ay_xn});  //增加
    //addIsolationToSingal(QPointF{ax_xn+0.0,ay_xn-5.0},QPointF{ax_xn+0.0,ay_xn+5.0},c1);
    //addIsolationToSingal(QPointF{endSII.rx()+0.0,ay_xn-5.0},QPointF{endSII.rx()+0.0,ay_xn+5.0},c1);
    int SII_XII_len=SI_XI_len;
    QPointF endXII(endSII.rx()+SII_XII_len*track_len_ratio,ay_xn);
    MyLineItem *mylineSIIXII = addLineToScene(endSII,endXII);
    this->tracktopstore->addCriticalPointsInformation("XII",QPointF{endSII.rx()+SII_XII_len*2.2*track_len_ratio,ay_xn});  //增加
    //addIsolationToSingal(QPointF{endXII.rx()+0.0,ay_xn-5.0},QPointF{endXII.rx()+0.0,ay_xn+5.0},c1);
    int length_XII_S = XI_SN_len;
    QPointF endS(endXI.rx()+length_XII_S*track_len_ratio,ay_xn);
    MyLineItem *lineXII_S = addLineToScene(endXII,endS);

    this->tracktopstore->addCriticalPointsInformation("S",endS);  //增加
    this->tracktopstore->addCriticalPointsInformation("S",QPointF{endS.rx()+120,endS.ry()});  //增加


    //addIsolationToSingal(QPointF{endS.rx()+0.0,ay_xn-5.0},QPointF{endS.rx()+0.0,ay_xn+5.0},c1);
    auto S_1102_len = SN_1101_len;
    QPointF end1102(endSN.rx()+S_1102_len*track_len_ratio,ay_xn);
    MyLineItem *mylineS1102 = addLineToScene(endS,end1102);
    this->tracktopstore->addCriticalPointsInformation("1102",end1102);  //增加

    addIsolationToSingal(QPointF{end1102.rx()+0.0,ay_xn-5.0},QPointF{end1102.rx()+0.0,ay_xn+5.0},c1);
    auto _1102_1104_len = _1101_1103_len;
    QPointF end1104(end1102.rx()+_1102_1104_len*track_len_ratio,ay_xn);
    MyLineItem *myline1102_1104 = addLineToScene(end1102,end1104);
    this->tracktopstore->addCriticalPointsInformation("1104",end1104);  //增加

    addIsolationToSingal(QPointF{end1104.rx()+0.0,ay_xn-5.0},QPointF{end1104.rx()+0.0,ay_xn+5.0},c1);
    auto _1104_1106_len =  _1103_1105_len;
    QPointF end1106(end1104.rx()+_1104_1106_len*track_len_ratio,ay_xn);
    MyLineItem *myline1104_1106 = addLineToScene(end1104,end1106);

    this->tracktopstore->addCriticalPointsInformation("1106",end1106);  //增加


    addIsolationToSingal(QPointF{end1106.rx()+0.0,ay_xn-5.0},QPointF{end1106.rx()+0.0,ay_xn+5.0},c1);
    auto _1106_1108_len =  _1105_1107_len;
    QPointF end1108(end1106.rx()+_1106_1108_len*track_len_ratio,ay_xn);
    MyLineItem *myline1106_1108 = addLineToScene(end1106,end1108);
    this->tracktopstore->addCriticalPointsInformation("1108",end1108);  //增加

    addIsolationToSingal(QPointF{end1108.rx()+0.0,ay_xn-5.0},QPointF{end1108.rx()+0.0,ay_xn+5.0},c1);
    auto length_1108_part1 =pi*innerradiance*0.8;
    QPointF end1108_part1(end1108.rx()+length_1108_part1*track_len_ratio,ay_xn);
    MyLineItem *myline1108_part1=addLineToScene(end1108,end1108_part1);

    this->tracktopstore->addCriticalPointsInformation("1108_left",end1108_part1);  //增加
    auto length_1108_part2 =pi*innerradiance*0.2;
    QPointF end1108_part2(startxn.rx()-length_1108_part2*track_len_ratio,ay_xn);
    MyLineItem *myline1108_part2=addLineToScene(startxn,end1108_part2);

    this->tracktopstore->addCriticalPointsInformation("1108_right",end1108_part2);  //增加 相对于内圈的

    dist_X_5=350;              //信号灯X到道岔5的距离
    dist_x_3 = dist_X_5+450;   //信号灯X到道岔3的距离
    QPointF  s5(ax_startx+dist_X_5*track_len_ratio,ay_startx);
    this->tracktopstore->addCriticalPointsInformation("5",s5);  //增加 表示道岔5的位置

    QPointF  s3(ax_startx+dist_x_3*track_len_ratio,ay_startx);
    this->tracktopstore->addCriticalPointsInformation("3",s3);  //增加  表示道岔3的位置

    QPointF  s1(ax_startx+dist_X_5*track_len_ratio,ay_xn);
    this->tracktopstore->addCriticalPointsInformation("1",s1);  //增加 表示道岔1的位置

    QPointF  s7(ax_startx+dist_x_3*track_len_ratio,ay_xn);
    this->tracktopstore->addCriticalPointsInformation("7",s7);  //增加 表示道岔7的位置

    QLineF s5_s7(s5,s7);
    QLineF s3_s1(s3,s1);
    mtoplogyscene->addItem(new MyLineItem(s5_s7));
    mtoplogyscene->addItem(new MyLineItem(s3_s1));
    dist_x_11 = dist_X_5+450+300;  //信号灯X到道岔11的距离
    QPointF  s11(ax_startx+dist_x_11*track_len_ratio,ay_startx);

    this->tracktopstore->addCriticalPointsInformation("11",s11);  //增加 表示道岔11的位置


    QPointF  s9(ax_startx+dist_x_11*track_len_ratio,ay_xn);

    this->tracktopstore->addCriticalPointsInformation("9",s9);  //增加 表示道岔9的位置


    auto dist_x_sxcg=dist_X_5+450+300+350;
    int sxgdao = tracktoplodgyview->height()/7;                  //假设上行轨道线离上行线距离
    QPointF  sxcg(ax_startx+dist_x_sxcg*track_len_ratio,ay_startx-sxgdao);  //上行侧轨开始点位置
    QLineF s11_sxcg (s11,sxcg);
    MyLineItem *myliness1_sxcg = new MyLineItem(s11_sxcg);
    mtoplogyscene->addItem(myliness1_sxcg);
    this->tracktopstore->addCriticalPointsInformation("上行侧轨开始处",sxcg);  //增加 表示上行侧轨开始地方


    sxcg_single_s3=QPointF(endSI.rx()+80, endSI.ry()-sxgdao);    //上行侧轨信号机S3的位置***
    QLineF sxcg_singles3(sxcg,sxcg_single_s3);
    MyLineItem *mylinessxcg_singles3= new MyLineItem(sxcg_singles3);
    mtoplogyscene->addItem(mylinessxcg_singles3);
    this->tracktopstore->addCriticalPointsInformation("上行侧轨S3处",sxcg_single_s3);  //增加



    //在上行线上的信号机S3出加入一个绝缘结，也就说将画图和实际要计算的位置分开
    addIsolationToSingal(QPointF{sxcg_single_s3.rx(),sxcg_single_s3.ry()+5},QPointF{sxcg_single_s3.rx(),sxcg_single_s3.ry()-5},c1);
    int lenght_S3_X3=400;
    endX3=QPointF(sxcg_single_s3.rx()+lenght_S3_X3*track_len_ratio,sxcg_single_s3.ry());
    QLineF S3_X3(sxcg_single_s3,endX3);
    MyLineItem *mylineS3X3 = new MyLineItem(S3_X3);  //默认的线段宽度是3个像素
    mtoplogyscene->addItem(mylineS3X3);

    this->tracktopstore->addCriticalPointsInformation("上行侧轨X3处",endX3);  //增加

    addIsolationToSingal(QPointF{endX3.rx(),endX3.ry()+5},QPointF{endX3.rx(),endX3.ry()-5},c1);
    //在X3位置后面加入100, 然后再SN位置减去200拉到
    int lenght_X3_SXCG_TAIL=200;     //离信号机X3的距离
    QPointF endSXCG_TAIL(endX3.rx()+lenght_X3_SXCG_TAIL*track_len_ratio,endX3.ry());
    QLineF S3_SXCG_TAIL(endX3,endSXCG_TAIL);
    MyLineItem *mylineX3SXCGTAIL = new MyLineItem(S3_SXCG_TAIL);  //默认的线段宽度是3个像素
    mtoplogyscene->addItem(mylineX3SXCGTAIL);

    this->tracktopstore->addCriticalPointsInformation("上行侧轨拖尾处",endSXCG_TAIL);  //增加



    //在SN的前面200位置，找到一个点成为道岔S4的位置
    QPointF S4(endSN.rx()-200*track_len_ratio+100,endSN.ry());
    QLineF SXCG_TAIL_S4(endSXCG_TAIL,S4);
    MyLineItem *mylineSXCG_TAIL_S4 = new MyLineItem(SXCG_TAIL_S4);  //默认的线段宽度是3个像素
    mtoplogyscene->addItem(mylineSXCG_TAIL_S4);

    this->tracktopstore->addCriticalPointsInformation("4",S4);  //增加道岔位置，记住到道岔接线处要预留一个小的部分



    qreal xxgdao = tracktoplodgyview->height()/7;  //假设上行轨道线离上行线距离
    QPointF  xxcg(sxcg.rx(),ay_xn+xxgdao);  //下行侧轨开始点位置 （最为重要的两个地方）
    QLineF s9_xxcg (s9,xxcg);
    MyLineItem *myliness9_xxcg= new MyLineItem(s9_xxcg);  //默认的线段宽度是3个像素
    mtoplogyscene->addItem(myliness9_xxcg);

    this->tracktopstore->addCriticalPointsInformation("下行侧轨开始处",xxcg);  //下行侧轨开始处是指从左手边往右看方向的


    //下行侧柜上的信号机S4的位置
    QPointF xxcg_single_s4(endSI.rx()+80, endSII.ry()+xxgdao);    //上行侧轨信号机S3的位置
    QLineF xxcg_single_s4_line (xxcg,xxcg_single_s4);
    MyLineItem *mylinesxxcg_singles4= new MyLineItem(xxcg_single_s4_line);  //默认的线段宽度是3个像素
    mtoplogyscene->addItem(mylinesxxcg_singles4);

    this->tracktopstore->addCriticalPointsInformation("下行侧轨S4处",xxcg_single_s4);  //下行侧轨开始处是指从左手边往右看方向的

    addIsolationToSingal(QPointF{endX3.rx(),ay_startx+5},QPointF{endX3.rx(),ay_startx-5},c1);
    QPointF xxcg_single_X4(endSI.rx()+180,endSII.ry()+xxgdao);
    QLineF lines_X4(xxcg_single_s4,xxcg_single_X4);
    mtoplogyscene->addItem(new MyLineItem(lines_X4));

    //this->tracktopstore->addCriticalPointsInformation("下行侧轨X4处",xxcg_single_X4);  //下行侧轨开始处是指从左手边往右看方向的

    int tepploc1=endSI.rx()+195;
    int tepploc2=endSII.ry()+xxgdao;

    this->tracktopstore->addCriticalPointsInformation("下行侧轨X4处",QPointF{(double)tepploc1,(double)tepploc2}); //下行侧轨开始处是指从左手边往右看方向的



    //在X4位置后面加入100, 然后再SN位置减去200拉到
    int lenght_X4_XXCG_TAIL=100;     //离信号机X3的距离
    QPointF endXXCG_TAIL(xxcg_single_X4.rx()+lenght_X4_XXCG_TAIL*track_len_ratio,xxcg_single_X4.ry());
    QLineF X4_XXCG_TAIL(xxcg_single_X4,endXXCG_TAIL);
    MyLineItem *mylineX4_XXCG_TAIL = new MyLineItem(X4_XXCG_TAIL);  //默认的线段宽度是3个像素
    mtoplogyscene->addItem(mylineX4_XXCG_TAIL);
    this->tracktopstore->addCriticalPointsInformation("上行侧轨拖尾处",endXXCG_TAIL);  //增加 从屏幕左边向右看
    //在转辙机S2的前面
    QPointF S2(endS.rx()-200*track_len_ratio+100,endS.ry());
    QLineF XXCG_TAIL_S4(endXXCG_TAIL,S2);
    MyLineItem *mylineXXCG_TAIL_S4 = new MyLineItem(XXCG_TAIL_S4);  //默认的线段宽度是3个像素
    mtoplogyscene->addItem(mylineXXCG_TAIL_S4);
    this->tracktopstore->addCriticalPointsInformation("2",S2);  //增加

    addTextTolocation("3G",QPointF{endX3.rx()-((lenght_S3_X3)/2)*track_len_ratio-10,endX3.ry()-20},QColor(255,255,0));
    addTextTolocation("1G",QPointF{endX3.rx()-((lenght_S3_X3)/2)*track_len_ratio-10,endX3.ry()-20+(tracktoplodgyview->height()/7)},QColor(255,255,0));
    addTextTolocation("IIG",QPointF{endX3.rx()-((lenght_S3_X3)/2)*track_len_ratio-10,endX3.ry()-20+(tracktoplodgyview->height()/7)*2},QColor(255,255,0));
    addTextTolocation("4G",QPointF{endX3.rx()-((lenght_S3_X3)/2)*track_len_ratio-10,endX3.ry()-20+(tracktoplodgyview->height()/7)*3},QColor(255,255,0));
    addTextTolocation("X1LQG",QPointF{end1101.rx()-((SN_1101_len)/2)*track_len_ratio-10,end1101.ry()+5},QColor(96,99,101));
    addTextTolocation("1101G",QPointF{end1103.rx()-((_1101_1103_len)/2)*track_len_ratio-20,end1103.ry()+5},QColor(96,99,101));
    addTextTolocation("1103G",QPointF{end1105.rx()-((_1103_1105_len)/2)*track_len_ratio-20,end1105.ry()+5},QColor(96,99,101));
    addTextTolocation("1105G",QPointF{end1107.rx()-((_1105_1107_len)/2)*track_len_ratio-20,end1107.ry()+5},QColor(96,99,101));
    addTextTolocation("1107G",QPointF{end1107.rx()+((length_1107_part1)/2)*track_len_ratio-20,end1107.ry()+5},QColor(96,99,101));
    addTextTolocation("1102G",QPointF{end1102.rx()-((S_1102_len)/2)*track_len_ratio-10,end1102.ry()-24},QColor(96,99,101));
    addTextTolocation("1104G",QPointF{end1104.rx()-((_1102_1104_len)/2)*track_len_ratio-20,end1104.ry()-24},QColor(96,99,101));
    addTextTolocation("1106G",QPointF{end1106.rx()-((_1104_1106_len)/2)*track_len_ratio-30,end1106.ry()-24},QColor(96,99,101));
    addTextTolocation("1108G",QPointF{end1108.rx()-((_1106_1108_len)/2)*track_len_ratio-20,end1108.ry()-24},QColor(96,99,101));
    addIsolationToSingal(QPointF{end1107.rx(),end1107.ry()-10.0},QPointF{end1107.rx(),end1107.ry()-10.0-single_button_height},c1);

    //
    addSingalToScene(sx_signal_1107_a,end1107.rx()+5,end1107.ry()-10-single_button_height,mycolor,"1107-B1");
    addSingalToScene(sx_signal_1107_b,end1107.rx()+5+single_button_width,end1107.ry()-10-single_button_height,mycolor,"1107-B2");

    addIsolationToSingal(QPointF{end1105.rx(),end1105.ry()-10.0},QPointF{end1105.rx(),end1105.ry()-10.0-single_button_height},c1);
    addSingalToScene(sx_signal_1105_a,end1105.rx()+5,end1105.ry()-10-single_button_height,mycolor,"1105-B1");
    addSingalToScene(sx_signal_1105_b,end1105.rx()+5+single_button_width,end1105.ry()-10-single_button_height,mycolor,"1105-B2");

    addIsolationToSingal(QPointF{end1103.rx(),end1103.ry()-10.0},QPointF{end1103.rx(),end1103.ry()-10.0-single_button_height},c1);
    addSingalToScene(sx_signal_1103_a,end1103.rx()+5,end1103.ry()-10-single_button_height,mycolor,"1103-B1");
    addSingalToScene(sx_signal_1103_b,end1103.rx()+5+single_button_width,end1103.ry()-10-single_button_height,mycolor,"1103-B2");

    addIsolationToSingal(QPointF{end1101.rx(),end1101.ry()-10.0},QPointF{end1101.rx(),end1101.ry()-10.0-single_button_height},c1);
    addSingalToScene(sx_signal_1101_a,end1101.rx()+5,end1101.ry()-10-single_button_height,mycolor,"1101-B1");
    addSingalToScene(sx_signal_1101_b,end1101.rx()+5+single_button_width,end1101.ry()-10-single_button_height,mycolor,"1101-B2");

    //SN-A1表示按钮
    addIsolationToSingal(QPointF{endSN.rx()+120,endSN.ry()-10.0},QPointF{endSN.rx()+120,endSN.ry()-10.0-single_button_height},c1);
    addButtonToScene(mybutton_SN_A1,"SN-A1",endSN.rx()+5+120,endSN.ry()-10-single_button_height);
    addButtonToScene(mybutton_SN_A2,"SN-A2",endSN.rx()+5+120+single_button_height,endSN.ry()-10-single_button_height);

    //
    addSingalToScene(sn_signal_b1,(endSN.rx()-5-single_button_width+120),endSN.ry()-10-single_button_height,mycolor,"SN-B1");
    addSingalToScene(sn_signal_b2,(endSN.rx()-5-single_button_width*2+120),endSN.ry()-10-single_button_height,mycolor,"SN-B2");

    //为了画得好看
    addSingalToScene(xi_signal_b1,endX3.rx()+5,endX3.ry()-10-single_button_height+sxgdao,mycolor,"XI-B1");
    addSingalToScene(xi_signal_b2,(endX3.rx()+5+single_button_width),endX3.ry()-10-single_button_height+sxgdao,mycolor,"XI-B2");
    addIsolationToSingal(QPointF{endX3.rx(),ay_startx-10.0},QPointF{endX3.rx(),ay_startx-10.0-single_button_height},c1);
    addButtonToScene(mybutton_XI_A1,"XI-A",endX3.rx()-5-single_button_width,ay_startx-10.0-single_button_height); //这里使用30是最好的距离

    addButtonToScene(mybutton_X_A1,"X-A1",ax_startx-20,ay_startx-10.0-single_button_height); //这里使用30是最好的距离
    addButtonToScene(mybutton_X_A2,"X-A2",ax_startx-20-single_button_width,ay_startx-10.0-single_button_height);
    addButtonToScene(mybutton_X_A3,"X-A3",ax_startx-20-single_button_width*2,ay_startx-10.0-single_button_height);
    addIsolationToSingal(QPointF{end1108.rx(),ay_xn+10.0},QPointF{end1108.rx(),ay_xn+10.0+single_button_height},c1);

    addSingalToScene(signal_1108_b1,end1108.rx()-20.0,ay_xn+10.0,mycolor,"1108-B1");
    addSingalToScene(signal_1108_b2,end1108.rx()-20.0-single_button_width,ay_xn+10.0,mycolor,"1108-B2");
    addIsolationToSingal(QPointF{end1106.rx(),ay_xn+10.0},QPointF{end1106.rx(),ay_xn+10.0+single_button_height},c1);

    addSingalToScene(signal_1106_b2,end1106.rx()-20.0,ay_xn+10.0,mycolor,"1106-B1");
    addSingalToScene(signal_1106_b1,end1106.rx()-20.0-single_button_width,ay_xn+10.0,mycolor,"1106-B2");
    addIsolationToSingal(QPointF{end1104.rx(),ay_xn+10.0},QPointF{end1104.rx(),ay_xn+10.0+single_button_height},c1);


    addSingalToScene(signal_1104_b1,end1104.rx()-20,ay_xn+10.0,mycolor,"1104-B1");
    addSingalToScene(signal_1104_b2,end1104.rx()-20-single_button_width,ay_xn+10.0,mycolor,"1104-B2");
    addIsolationToSingal(QPointF{end1102.rx(),ay_xn+10.0},QPointF{end1102.rx(),ay_xn+10.0+single_button_height},c1);


    addSingalToScene(signal_1102_b1,end1102.rx()-20,ay_xn+10.0,mycolor,"1102-B1");
    addSingalToScene(signal_1102_b2,end1102.rx()-20-single_button_width,ay_xn+10.0,mycolor,"1102-B2");


    addIsolationToSingal(QPointF{endSN.rx()+120,ay_xn+10.0},QPointF{endSN.rx()+120,ay_xn+10.0+single_button_height},c1);
    addSingalToScene(signal_s_b1,endSN.rx()+120-5-single_button_width,ay_xn+10.0,mycolor,"S-B1");
    addSingalToScene(signal_s_b2,endSN.rx()+120-5-single_button_width*2,ay_xn+10.0,mycolor,"S-B2");
    addButtonToScene(mybutton_s_A1,"S-A1",endSN.rx()+120+5.0,ay_xn+10.0);
    addButtonToScene(mybutton_s_A2,"S-A2",endSN.rx()+120+5+single_button_width,ay_xn+10.0);
    addButtonToScene(mybutton_s_A3,"S-A3",endSN.rx()+120+5+single_button_width*2,ay_xn+10.0);

    //为了画得好看
    addIsolationToSingal(QPointF{endX3.rx(),endXII.ry()-10},QPointF{endX3.rx(),endXII.ry()-10-single_button_height},c1);
    addSingalToScene(signal_xii_b1,endX3.rx()+5.0,endXII.ry()-10-single_button_height,mycolor,"XII-B1");
    addSingalToScene(signal_xii_b2,endX3.rx()+5.0+single_button_width,endXII.ry()-10-single_button_height,mycolor,"XII-B2");
    addButtonToScene(mybutton_xii_A1,"XII-A",endX3.rx()-5-single_button_width,endXII.ry()-10-single_button_height);


    //这几个特殊处理
    addIsolationToSingal(QPointF{sxcg_single_s3.rx(),ay_xn+10.0+single_button_height},QPointF{sxcg_single_s3.rx(),ay_xn+10.0},c1);
    addSingalToScene(signal_sii_b1,sxcg_single_s3.rx()-20,ay_xn+10.0,mycolor,"SII-B1");
    addSingalToScene(signal_sii_b2,sxcg_single_s3.rx()-20-single_button_width,ay_xn+10.0,mycolor,"SII-B2");
    addButtonToScene(mybutton_sii_A1,"SII-A",sxcg_single_s3.rx()+5,ay_xn+10.0);

    addIsolationToSingal(QPointF{startxn.rx(),ay_xn+10.0},QPointF{startxn.rx(),ay_xn+10.0+single_button_height},c1);
    addSingalToScene(signal_XN_b1,startxn.rx()+5,ay_xn+10.0,mycolor,"XN-B1");
    addSingalToScene(signal_XN_b2,startxn.rx()+5+single_button_width,ay_xn+10.0,mycolor,"XN-B2");
    addButtonToScene(mybutton_xn_A1,"XN-A1",startxn.rx()-5-single_button_width,ay_xn+10);
    addButtonToScene(mybutton_xn_A2,"XN-A2",startxn.rx()-5-single_button_width*2,ay_xn+10);

    //为了画得好看
    addIsolationToSingal(QPointF{endX3.rx(),xxcg_single_X4.ry()-10},QPointF{endX3.rx(),xxcg_single_X4.ry()-10-single_button_height},c1);
    addSingalToScene(signal_X4_b1,endX3.rx()+5,xxcg_single_X4.ry()-10-single_button_height,mycolor,"X4-B1");
    addSingalToScene(signal_X4_b2,endX3.rx()+5+single_button_width,xxcg_single_X4.ry()-10-single_button_height,mycolor,"X4-B2");
    addButtonToScene(mybutton_x4_A1,"X4-A",endX3.rx()-5-single_button_width,xxcg_single_X4.ry()-10-single_button_height);

    addIsolationToSingal(QPointF{xxcg_single_s4.rx(),xxcg_single_s4.ry()+10+single_button_height},QPointF{xxcg_single_s4.rx(),xxcg_single_s4.ry()+10},c1);
    addSingalToScene(signal_S4_b1,xxcg_single_s4.rx()-5-single_button_width,xxcg_single_s4.ry()+10,mycolor,"S4-B1");
    addSingalToScene(signal_S4_b2,xxcg_single_s4.rx()-5-single_button_width*2,xxcg_single_s4.ry()+10,mycolor,"S4-B2");
    addButtonToScene(mybutton_s4_A1,"S4-A",xxcg_single_s4.rx()+5,xxcg_single_s4.ry()+10);

    //下面这几个特殊处理
    addIsolationToSingal(QPointF{sxcg_single_s3.rx(),endSI.ry()+10+single_button_height},QPointF{sxcg_single_s3.rx(),endSI.ry()+10},c1);


    addSingalToScene(signal_SI_b1,sxcg_single_s3.rx()-5-single_button_width,endSI.ry()+10,mycolor,"SI-B1");
    addSingalToScene(signal_SI_b2,sxcg_single_s3.rx()-5-single_button_width*2,endSI.ry()+10,mycolor,"SI-B2");
    addIsolationToSingal(QPointF{endX3.rx(),endX3.ry()-10-single_button_height},QPointF{endX3.rx(),endX3.ry()-10},c1);
    addButtonToScene(mybutton_si_A1,"SI-A",sxcg_single_s3.rx()+5.0,endSI.ry()+10);


    addSingalToScene(signal_X3_b1,endX3.rx()+5,endX3.ry()-10-single_button_height,mycolor,"X3-B1");
    addSingalToScene(signal_X3_b2,endX3.rx()+5+single_button_width,endX3.ry()-10-single_button_height,mycolor,"X3-B2");
    addButtonToScene(mybutton_x3_A1,"X3-A",endX3.rx()-5-single_button_width,endX3.ry()-10-single_button_height);

    addIsolationToSingal(QPointF{sxcg_single_s3.rx(),sxcg_single_s3.ry()+10+single_button_height},QPointF{sxcg_single_s3.rx(),sxcg_single_s3.ry()+10},c1);

    addSingalToScene(signal_S3_b1,sxcg_single_s3.rx()-5-single_button_width,sxcg_single_s3.ry()+10,mycolor,"S3-B1");
    addSingalToScene(signal_S3_b2,sxcg_single_s3.rx()-5-single_button_width*2,sxcg_single_s3.ry()+10,mycolor,"S3-B2");
    addButtonToScene(mybutton_S3_A1,"S3-A",sxcg_single_s3.rx()+5,sxcg_single_s3.ry()+10);
    addIsolationToSingal(QPointF{endX3.rx(),ay_xn-5},QPointF{endX3.rx(),ay_xn+5},c1);
}

void MainWindow::configureOutterTrack(){

    this->outterradiance  = 520;
    this->innerradiance = 400;
    this->width_ration_for_track=0.8;     //期望在多大的场景宽度比列上画轨道
    this->totallen = 3254;
    this->pi =3.1415926;
    auto directedtraccklen= (this->totallen-2*outterradiance); //水平方向上的轨道长度
    this->outtracklen = directedtraccklen*2.0+2.0*(this->pi)*(this->outterradiance);     //外圈长度
    this->innertracklen = directedtraccklen*2.0+2.0*(this->pi)*(this->innerradiance); //内圈长度
    track_len_ratio = tracktoplodgyview->width()*width_ration_for_track/outtracklen; //每一个像素长度代表有多长的铁路轨道
    XI_SN_len = 600.0;                    //XI到SN的距离是600米
    SI_XI_len = 400.0;                    //从SI到XI的距离是400米
    X_SI_len = directedtraccklen-SI_XI_len-XI_SN_len;
    ax_startx = -(tracktoplodgyview->width()/2)+0.1*tracktoplodgyview->width();    //上行轨道的出发点X在场景坐标系中的X坐标
    ay_startx = -(tracktoplodgyview->height()/2.0)+(tracktoplodgyview->height()/7)*3.0; //上行轨道的出发点X在场景坐标系中的Y坐标
    endSI=QPointF{ax_startx+(X_SI_len)*track_len_ratio,ay_startx};
    MyLineItem *line_X_SI=addLineToScene(QPointF{ax_startx,ay_startx},QPointF{ax_startx+(X_SI_len)*track_len_ratio,ay_startx});

    this->tracktopstore->addCriticalPointsInformation("X",QPointF{ax_startx,ay_startx});  //增加
    //this->tracktopstore->addCriticalPointsInformation("SI",endSI);  //增加
    int si_11_distance=390;//通过调节这个距离值，使得界面上的画图能够接近真实值
    this->tracktopstore->addCriticalPointsInformation("SI",QPointF{endSI.rx()+(si_11_distance)*track_len_ratio,endSI.ry()});

    addIsolationToSingal(QPointF{ax_startx-0.0,ay_startx-5.0},QPointF{ax_startx-0.0,ay_startx+5.0},c1);
    //addIsolationToSingal(QPointF{ax_startx+(X_SI_len)*track_len_ratio-0.0,ay_startx-5.0},QPointF{ax_startx+(X_SI_len)*track_len_ratio-0.0,ay_startx+5.0},c1);
    MyLineItem *line_SI_XI=addLineToScene(QPointF{ax_startx+(X_SI_len+SI_XI_len)*track_len_ratio,ay_startx},QPointF{ax_startx+(X_SI_len)*track_len_ratio,ay_startx});
    endXI=QPointF{ax_startx+(X_SI_len+SI_XI_len)*track_len_ratio,ay_startx};

    //这里不增加2.2倍，绘制进路不正确
    QPointF tempXILoc = QPointF{ax_startx+(X_SI_len+SI_XI_len*2)*track_len_ratio,ay_startx};
    //this->tracktopstore->addCriticalPointsInformation("XI",QPointF{ax_startx+(X_SI_len+SI_XI_len*2.2)*track_len_ratio,ay_startx});  //增加
    this->tracktopstore->addCriticalPointsInformation("XI",tempXILoc);  //增加

    //addIsolationToSingal(QPointF{endXI.rx(),ay_startx-5.0},QPointF{endXI.rx(),ay_startx+5.0},c1);
    int length_XI_SN = 600;
    endSN=QPointF{endXI.rx()+length_XI_SN*track_len_ratio,ay_startx};
    MyLineItem *line_XI_SN=addLineToScene(QPointF{endXI.rx()+length_XI_SN*track_len_ratio,ay_startx},endXI);
    //addIsolationToSingal(QPointF{endSN.rx(),ay_startx-5.0},QPointF{endSN.rx(),ay_startx+5.0},c1);

    this->tracktopstore->addCriticalPointsInformation("SN",QPointF{endXI.rx()+length_XI_SN*track_len_ratio*2.2,ay_startx});  //增加


    SN_1101_len = this->pi*(this->outterradiance);
    end1101=QPointF{endSN.rx()+SN_1101_len*track_len_ratio,ay_startx};
    MyLineItem *line_SN_1101=addLineToScene(endSN,end1101);
    this->tracktopstore->addCriticalPointsInformation("1101",end1101);  //增加

    addIsolationToSingal(QPointF{end1101.rx(),ay_startx-5.0},QPointF{end1101.rx(),ay_startx+5.0},c1);
    _1103_1105_len = 716.67;
    _1101_1103_len = (directedtraccklen-_1103_1105_len)/2;
    end1103=QPointF{end1101.rx()+_1101_1103_len*track_len_ratio,ay_startx};
    MyLineItem *line_1101_1103 =addLineToScene(end1101,end1103);

    this->tracktopstore->addCriticalPointsInformation("1103",end1103);  //增加


    addIsolationToSingal(QPointF{end1103.rx(),ay_startx-5.0},QPointF{end1103.rx(),ay_startx+5.0},c1);
    end1105=QPointF(end1103.rx()+_1103_1105_len*track_len_ratio,ay_startx);
    MyLineItem *line_1103_1105=addLineToScene(end1103,end1105);

    this->tracktopstore->addCriticalPointsInformation("1105",end1105);  //增加


    addIsolationToSingal(QPointF{end1105.rx(),ay_startx-5.0},QPointF{end1105.rx(),ay_startx+5.0},c1);
    _1105_1107_len =(directedtraccklen-_1103_1105_len)/2;
    end1107=QPointF(end1105.rx()+_1105_1107_len*track_len_ratio,ay_startx);
    MyLineItem *line_1105_1107=addLineToScene(end1105,end1107);

    this->tracktopstore->addCriticalPointsInformation("1107",end1107);  //增加

    addIsolationToSingal(QPointF{end1107.rx(),ay_startx-5.0},QPointF{end1107.rx(),ay_startx+5.0},c1);
    length_1107_part1 =(this->pi)*(this->outterradiance)*0.8;
    QPointF end1107_part1(end1107.rx()+length_1107_part1*track_len_ratio,ay_startx);
    MyLineItem *line_1107_part1= addLineToScene(end1107,end1107_part1);

    this->tracktopstore->addCriticalPointsInformation("1107_right",end1107_part1);  //在要设置

    auto _1107_part2_len =(this->pi)*(this->outterradiance)*0.2;
    QPointF end1107_part2(ax_startx-_1107_part2_len*track_len_ratio,ay_startx);
    MyLineItem* line_1107_part2= addLineToScene(end1107_part2,QPointF{ax_startx+0.0,ay_startx+0.0});

    this->tracktopstore->addCriticalPointsInformation("1107_left",end1107_part2);  //相对于startx的1107的右边，与1107_left组成循环
    //记录所有这些位置是为了重新绘制状态
}


//应该是编译环境没有配置好才导致的错误

void MainWindow::initialScene(){
    ui->setupUi(this);
    showMaximized();
    scene_size = qApp->primaryScreen()->availableSize();
    this->setFixedSize(scene_size);
    setWindowFlags(Qt::WindowCloseButtonHint );
    this->tracktoplodgyview = new QGraphicsView(ui->centralwidget);
    mtoplogyscene= new ToplogyScene(tracktoplodgyview);
    tracktoplodgyview->setGeometry(QRect(0,scene_size.height()-scene_size.height()/1.09, scene_size.width(), scene_size.height()/1.15));  //QGraphicsView的大小和位置不随主窗口变化？？？
    tracktoplodgyview->setScene(mtoplogyscene);
    tracktoplodgyview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tracktoplodgyview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tracktoplodgyview->setRenderHint(QPainter::Antialiasing);
    tracktoplodgyview->setStyleSheet("border:none; background:black;");
    mtoplogyscene->setSceneRect(0-(tracktoplodgyview->width()/2),0-(tracktoplodgyview->height()/2),tracktoplodgyview->width(),tracktoplodgyview->height());  //设置中心点的坐标，然后要找出内圈和外圈半圆的位置，都以中心来定位

    tracktoplodgyview->show();
    single_button_width =15;
    single_button_height=15;
    heightsequal = 7.0;
    //qDebug() << "场景宽度......:" << tracktoplodgyview->width();
    //qDebug() << "场景高度......:" << tracktoplodgyview->height();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        // 自动读取串口号添加到端口portBox中
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            ui->comboBox->addItem(info.portName());
            serial.close();
        }
    }
}

MyLineItem* MainWindow::addLineToScene(QPointF start, qreal len,qreal track_len_ratio){
    QPointF end(start.rx()+len*track_len_ratio,start.ry());
    MyLineItem *line =addLineToScene(start,end);
    return line;
}


void MainWindow::addGroupToscene(CustomItemGroup *testgroup, qreal ax, qreal ay, QString textinfo){

    testgroup = new CustomItemGroup();
    testgroup->setHandlesChildEvents(false);   //让子项目自己处理,给一个横向排列的方式
    CustomGraphicsArrow * myarrow_left = new CustomGraphicsArrow();  //正方向箭头
    QPointF arrowsource(0,10);
    QPointF arrowdest(55,10);
    myarrow_left->setSourceAndDestPoint(arrowsource,arrowdest);
    testgroup->addToGroup(myarrow_left);
    auto * myarrow_right = new CustomGraphicsArrow(); //反方向箭头
    QPointF arrowsource1(120,10);
    QPointF arrowdest1(65,10);
    myarrow_right->setSourceAndDestPoint(arrowsource1, arrowdest1);
    testgroup->addToGroup(myarrow_right);
    //画一个圆环
    MyEllipseItem *myeclipse = new MyEllipseItem(140, 0, single_button_height, single_button_height);
    testgroup->addToGroup(myeclipse);
    //画一个矩形框
    QRectF myrect(180,0,single_button_height,single_button_height);
    MyRectItem * myrectitem = new MyRectItem(myrect);
    testgroup->addToGroup(myrectitem);

    QGraphicsTextItem *textItemX=new QGraphicsTextItem;
    textItemX->setPlainText(textinfo);
    textItemX->setFont(QFont("华文楷体",10));
    textItemX->setDefaultTextColor(QColor(255,255,255));
    textItemX->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable);
    textItemX->setPos(50,30);        //在两个箭头的中间
    testgroup->addToGroup(textItemX);

    QGraphicsTextItem *textItemqujian=new QGraphicsTextItem;
    textItemqujian->setPlainText("区间");
    textItemqujian->setFont(QFont("华文楷体",10));
    textItemqujian->setDefaultTextColor(QColor(255,255,255));
    textItemqujian->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable);
    textItemqujian->setPos(135,30);        //在两个箭头的中间
    testgroup->addToGroup(textItemqujian);


    QGraphicsTextItem *textItemgaifang=new QGraphicsTextItem;
    textItemgaifang->setPlainText("改方");
    textItemgaifang->setFont(QFont("华文楷体",10));
    textItemgaifang->setDefaultTextColor(QColor(255,255,255));
    textItemgaifang->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable);
    textItemgaifang->setPos(175,30);        //在两个箭头的中间
    testgroup->addToGroup(textItemgaifang);


    testgroup->setPos(ax,ay);    //调整大小和位置，表示在场景中的坐标位置，场景坐标的中心点是(0,0),
    mtoplogyscene->addItem(testgroup);
    mtoplogyscene->addRect(testgroup->boundingRect());
}


//需要判断加入的是哪个区段
MyLineItem * MainWindow::addLineToScene(QPointF start, QPointF end){
    QLineF line(start,end);
    MyLineItem *mylineline = new MyLineItem(line);  //默认的线段宽度是3个像素
    mtoplogyscene->addItem(mylineline);
    return mylineline;
}


//在主界面中更新这个区域
void MainWindow::slotsingalColorSpecfied(QString item_name,QString color){

    mtoplogyscene->update(this->signal2rect[item_name]);
    //在这里应该加入控制逻辑
    mtoplogyscene->driverSignalUpdate(item_name,color);

}

void MainWindow::addSingalToScene(Mysignal *mysingal, qreal aleft, qreal atop,QColor color,QString signalname){
    QRectF rect(aleft,atop,single_button_width,single_button_height);
    mysingal = new Mysignal(rect, mtoplogyscene,color);
    mysingal->setSignalName(signalname);
    mtoplogyscene->addItem(mysingal);
    signal2rect[signalname]=rect;
    connect(mtoplogyscene,&ToplogyScene::signalShouldDisplayStatus, mysingal, &Mysignal::slotsingalColorSpecfied);
    connect(mtoplogyscene,&ToplogyScene::signalShouldDisplayStatus, this, &MainWindow::slotsingalColorSpecfied);

}

//在这个里面加入拓扑信息进去，供联锁逻辑控制器使用
void  MainWindow::addButtonToScene(ButtonWidget *button, QString button_name, qreal aleft, qreal atop){

    button= new ButtonWidget(); //自定义的ActorWidget,我们后面根据位置来存储这些指针
    button->setButtonName(button_name);
    if(button_name=="X-A2" || button_name=="SN-A2" || button_name=="XN-A2" ||   button_name=="S-A2"  ){
        button->setStyleSheet("background-color: rgb(0,0,255)");
    }else{
        button->setStyleSheet("background-color: rgb(0,255,0)");
    }
    button->setSizePolicy( QSizePolicy::Minimum , QSizePolicy::Preferred );
    button->setFixedWidth(single_button_width);    //非要把这个设置小才行
    button->setFixedHeight(single_button_height);
    QGraphicsProxyWidget *proxy_xi= mtoplogyscene->addWidget(button); //在场景中添加自定义的ActorWidget，返回一个QGraphicsProxyWidget指针
    proxy_xi->setGeometry(QRectF(aleft,atop,single_button_height,single_button_height));
    //qDebug()<<"huangwei...."<<aleft<<"...."<<atop;
    QString temp_x;
    temp_x.setNum(aleft);
    QString temp_y;
    temp_y.setNum(atop);
    QString loc= temp_x+"|"+temp_y;
    this->tracktopstore->addItemToMap(button_name,loc);
    proxy_xi->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    proxy_xi->setFlags(QGraphicsItem::ItemIsFocusable); //通过QGraphicsProxyWidget指针设置其可选、可移动、可聚焦
    //每当鼠标被按下之后，我们可以打开计数器重新让其闪烁, 这个地方控制逻辑
    connect(mtoplogyscene,&ToplogyScene::buttonTargetName,button, &ButtonWidget::slotButtonTarget);
    connect(mtoplogyscene,&ToplogyScene::buttonTargetcalcelselect,button, &ButtonWidget::slotcacelbuttonselect);  //取消选择
    connect(mtoplogyscene,&ToplogyScene::shutdownTrainsignal,button, &ButtonWidget::slotBUttonStopFlash);
    //还需要关联进路不对应情况
    //由于某些按钮会发出进路请求，因此，我们将其反向绑定进来
    connect(button,&ButtonWidget::routeRequestMessage,mtoplogyscene, &ToplogyScene::requestRouteHandler);
    connect(button,&ButtonWidget::routecacleRequestMessage,mtoplogyscene, &ToplogyScene::cacelrequeRouteHanler);
    connect(button,&ButtonWidget::popUpDialogMessage,this,&MainWindow::popUpDiaglog);
}



//在横轴方向加入绿色条带，绿色表示定向方向
void MainWindow::addGreenLinetoSwitch(QPointF &sp,ToplogyScene *_mtoplogyscene,QColor c){

    QPointF startF(sp.rx()-7,sp.ry());
    QPointF endF(sp.rx()+7,sp.ry());
    QLineF line1(startF,endF);
    MyLineItem *Sline1 = new MyLineItem(line1);  //默认的线段宽度是3个像素
    //    QColor c;
    //    c.setRed(0);
    //    c.setGreen(255);
    //    c.setBlue(0);
    Sline1->setColor(c);
    _mtoplogyscene->addItem(Sline1);
}

//这个最后再弄
void  MainWindow::addFunctionButtons(QSize size){


    //因为清除不是特定的功能按钮，所以不做特殊处理的
    MenuPushButton* but = new MenuPushButton("总人解",this);
    but->setGeometry(size.rheight(), size.height()-90, 60, 40);  //关联它们的动作,到后面在排列
    //connect(but,&QPushButton::click, but, &MenuPushButton::buttonClicked);
    MenuPushButton* but1 = new MenuPushButton("总定位",this);
    but1->setGeometry(size.rheight()+60, size.height()-90, 60, 40);  //关联它们的动作
    MenuPushButton* but2 = new MenuPushButton("总反位",this);
    but2->setGeometry(size.rheight()+60*2, size.height()-90, 60, 40);  //关联它们的动作
    MenuPushButton* but3 = new MenuPushButton("总取消",this);
    but3->setGeometry(size.rheight()+60*3, size.height()-90, 60, 40);  //关联它们的动作
    MenuPushButton* but4 = new MenuPushButton("总故解",this);
    but4->setGeometry(size.rheight()+60*4, size.height()-90, 60, 40);  //关联它们的动作

    MenuPushButton* but6 = new MenuPushButton("单锁",this);
    but6->setGeometry(size.rheight()+60*6, size.height()-90, 60, 40);  //关联它们的动作
    MenuPushButton* but7 = new MenuPushButton("单解",this);
    but7->setGeometry(size.rheight()+60*7, size.height()-90, 60, 40);  //关联它们的动作
    MenuPushButton* but8 = new MenuPushButton("道岔封锁",this);
    but8->setGeometry(size.rheight()+60*8, size.height()-90, 60, 40);  //关联它们的动作
    MenuPushButton* but9 = new MenuPushButton("引导总锁",this);
    but9->setGeometry(size.rheight()+60*9, size.height()-90, 60, 40);  //关联它们的动作
    MenuPushButton* but10 = new MenuPushButton("道岔解封",this);
    but10->setGeometry(size.rheight()+60*10, size.height()-90, 60, 40);  //关联它们的动作
    // MenuPushButton* but11 = new MenuPushButton("上电解锁",this);
    //but11->setGeometry(size.rheight()+60*11, size.height()-70, 60, 40);  //关联它们的动作


    QPushButton* but5 = new QPushButton("清除",this);
    but5->setGeometry(size.rheight()+60*5, size.height()-90, 60, 40);  //关联它们的动作
    but5->setStyleSheet("background-color: rgb(115, 115, 115);");
    connect(but5, &QPushButton::clicked,but, &MenuPushButton::exterClearButtonClicked);

}
void MainWindow::addTextTolocation(QString text, QPointF pos, QColor color){
    QGraphicsTextItem *textItem1G=new QGraphicsTextItem;
    textItem1G->setPlainText(text);
    textItem1G->setFont(QFont("华文楷体",10));
    textItem1G->setDefaultTextColor(color);
    textItem1G->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable);
    textItem1G->setPos(pos);        //在S3和X3之间
    mtoplogyscene->addItem(textItem1G);
}

void MainWindow::addIsolationToSingal(QPointF start, QPointF end, QColor color){

    QLineF line(start,end);
    MyLineItem *placer = new MyLineItem(line);              //默认的线段宽度是3个像素
    placer->setColor(color);
    mtoplogyscene->addItem(placer);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slottrianlgle(int point ){
    qDebug() << "mytestvalues is obtained in mainwindows......" << point;
}

void MainWindow::SegmentBlockCirculDiagramBack(){
    displayMainwindow();
}

void MainWindow::showSegmentBlockCirculDiagram(){
    mStackedWidget->setCurrentWidget(this->msegmentcidigmwidget);
    //将所有其它按钮都设置为不可见

}

void MainWindow::showRequest(const QString &s)
{
    ui->m_trafficLabel->setText(tr("Traffic, transaction #%1:"
                               "\n\r-request: %2"
                                 )
                                .arg(++m_transactionCount)
                                .arg(s));

}

//in this function, we call fama once the segment status are changed
void MainWindow::updatesceneandreal(QByteArray ba){
    //this->mtoplogyscene->interlockingLogicProcess();
}

void MainWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.save();
    QRect imageRect = event->region().boundingRect();
    painter.restore();
}


