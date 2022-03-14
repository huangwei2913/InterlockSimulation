#include "segmentcircuitdiagram.h"
#include "ui_segmentcircuitdiagram.h"
#include <QQuickView>
#include <QPalette>
#include <QPushButton>
#include <QDebug>
#include <QMetaObject>
#include <QQmlProperty>

#include <QGuiApplication>
#include <QQmlProperty>
#include <QQuickView>
#include <QQuickItem>
#include <QMetaObject>
#include <QDebug>
#include <QMessageBox>
//混合QML和Qtwidget编程

SegmentCircuitDiagram
::SegmentCircuitDiagram
(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SegmentCircuitDiagram
       )
{
    ui->setupUi(this);
    qmlfileList.append("./blockingsegmentcd.qml");
    qmlfileList.append("./1txguzhang.qml");
    qmlfileList.append("./2txguzhang.qml");
    qmlfileList.append("./3txguzhang.qml");
    the_next_clicked = 0;
    myview = new QQuickView();
    myview->setSource(QUrl(QStringLiteral("./blockingsegmentcd.qml")));
    container = QWidget::createWindowContainer(myview, this);
    container->setFocusPolicy(Qt::TabFocus);
    btn_room_setting = new QPushButton(container);
    btn_room_setting->setGeometry(0, 0, 200, 28);
    btn_room_setting->setSizePolicy(QSizePolicy::Minimum , QSizePolicy::Preferred );
    btn_room_setting->setFixedWidth(200);    //非要把这个设置小才行
    btn_room_setting->setFixedHeight(28);
    btn_room_setting->setText("操作提示：选择故障点单击设置故障");         //在这个里面实现返回主菜单的功能
    btn_room_setting->setObjectName("btn_room_setting");
    ui->verticalLayout->addWidget(btn_room_setting);
    ui->verticalLayout->addWidget(container);
    //connect( ui->pushButton, &QPushButton::clicked, this, &SegmentCircuitDiagram::on_pushButtonConnect_clicked);
    setLayout(ui->verticalLayout);      //让layout随本窗口大小一起变化，这一句是必须的
    container->showMaximized();
    connect(ui->pushButton, &QPushButton::clicked, this, &SegmentCircuitDiagram::emitbacktomainsignal);
    //我们需要在qml中使用C++对象进行扩展，通过Q_INVOKABLE宏标记的public函数可以在QML中访问
    //通过C++来以便读取和写入属性，调用方法和接收信号通知
    //明天要搞这个东西https://blog.csdn.net/gongjianbo1992/article/details/87965925
    QObject *qmlObj= myview->rootObject();
    qDebug()<<"Cpp get qml property height"<<qmlObj->property("height");

    QObject *qmlRect=qmlObj->findChild<QObject*>("my_button1");
    if(qmlRect){
            qDebug()<<"Cpp get clicked_num"<<qmlRect->property("clcked_num");
      }

     //再来看下从qml发出信号，到C++接收，为此，我们需要创建一个cppObject对象管理qml发出的消息
     this->cppObj = new CppObject(this);
     connect(qmlObj,SIGNAL(qmlSendMsg(QString,QString)),cppObj,SLOT(cppRecvMsg(QString,QString)));

     //然后，将cppObj所发出的对象，关联到文本控件中作为提醒
     connect(cppObj,&CppObject::dispalyguzhansignal,ui->label,&QLabel::setText);

     //将故障下发功能加入进去
     connect(ui->pushButton_3,&QPushButton::clicked, this, &SegmentCircuitDiagram::sendMessageToxiaweiji);

     //设置下一块板的时候就是切换下一个qml???
     connect(ui->pushButton_4,&QPushButton::clicked, this, &SegmentCircuitDiagram::setNextGuzhangBan);

     //全复位故障
     connect(ui->pushButton_2, &QPushButton::clicked, this,&SegmentCircuitDiagram::resetAllGuzhang);

     //单点复位
     connect(ui->pushButton_5, &QPushButton::clicked, this,&SegmentCircuitDiagram::signalFaultSet);

     //
     for(int i=1;i<=300;i++){
          ui->comboBox->addItem(QString::number(i));
     }


}

//修复某个故障
void SegmentCircuitDiagram::signalFaultSet(){
    auto s = ui->comboBox->currentText();
    if(s==""){
       return;
    }
    if (isDigitStr(s)!=0){
        return;
    }
    //单个点故障复位
    int guzhangdian = s.toInt();
    qDebug()<<"Cguzhangdian..."<<guzhangdian;

    //首先找是哪块板子


    int in_which_block = (guzhangdian)/32;      //找到第几块板

    unsigned char qswei = 0xe0+in_which_block+1;


    int in_the_block_pos = guzhangdian%32;
    qDebug()<<"in_the_block_pos......"<<in_the_block_pos;

     unsigned char sjwei = ((guzhangdian-1)%32)&0x1f | 0xc0;

    if (in_the_block_pos==31){      //31个特殊处理就行
        sjwei = 0xdf;
    }

    qDebug()<<"qswei......"<<qswei<<"......sjwei"<<sjwei;
    QByteArray ba;
    ba.resize(2);
    ba[0] = qswei;
    ba[1] = sjwei;
    uint16_t wcrc = ModbusCRC16(ba);
    QByteArray resba;
    qDebug()<<"CRC码：低字节:"<<uint8_t(wcrc)<<" 高字节:"<<uint8_t(wcrc>>8);
    resba.resize(4);
    unsigned char di=uint8_t(wcrc);
    unsigned char gao=uint8_t(wcrc>>8);
    resba[0]=qswei;
    resba[1] = sjwei;
    resba[2] = di;
    resba[3] = gao;
    emit sendMessageToseriesport(resba);


}

void SegmentCircuitDiagram::resetAllGuzhang(){
    qDebug()<<"全复位故障已点击。。。";
    unsigned char qswei = 0xe0;
    unsigned char sjwei = 0xe0;
    QByteArray ba;
    ba.resize(2);
    ba[0] = qswei;
    ba[1] = sjwei;

    uint16_t wcrc = ModbusCRC16(ba);
    QByteArray resba;
    qDebug()<<"CRC码：低字节:"<<uint8_t(wcrc)<<" 高字节:"<<uint8_t(wcrc>>8);
    resba.resize(4);
    unsigned char di=uint8_t(wcrc);
    unsigned char gao=uint8_t(wcrc>>8);
    resba[0]=  ba[0];
    resba[1] = ba[1];
    resba[2] = di;
    resba[3] = gao;

    emit sendMessageToseriesport(resba);



}


void SegmentCircuitDiagram::setNextGuzhangBan(){
    the_next_clicked++;
    int index_in_list =(int)((int)the_next_clicked%4);
    QString tempaa = qmlfileList.at(index_in_list);
    myview->setSource(tempaa);
    QObject *qmlObj= myview->rootObject();
    QObject *qmlRect=qmlObj->findChild<QObject*>("my_button1");
    if(qmlRect){
            qDebug()<<"Cpp get clicked_num..................."<<qmlRect->property("clcked_num");
      }
    this->cppObj = new CppObject(this);
    connect(qmlObj,SIGNAL(qmlSendMsg(QString,QString)),cppObj,SLOT(cppRecvMsg(QString,QString)));
    //然后，将cppObj所发出的对象，关联到文本控件中作为提醒
    connect(cppObj,&CppObject::dispalyguzhansignal,ui->label,&QLabel::setText);
    container->update();
    ui->label->setText("");
}


//这一块是对的
void SegmentCircuitDiagram::sendMessageToxiaweiji(){

    if(ui->label->text()==""){
        return;
    }
    QString str=ui->label->text();
    if(str.contains("取消")){
            qDebug()<<"str...................."<<str;
        return;}

    QStringList strList = str.split(" ");
    if(strList.length()!=3){return;}
    int guzhangdian = strList[1].toInt();
    qDebug()<<"Cguzhangdian..."<<guzhangdian;

    int in_which_block = (guzhangdian)/32;      //找到第几块板
    qDebug()<<"in_which_block......"<<in_which_block;

    unsigned char qswei = 0xe0+in_which_block+1;

    //这里要特殊处理一下31这个故障点的
    int in_the_block_pos = guzhangdian%32;
    qDebug()<<"in_the_block_pos......"<<in_the_block_pos;

    unsigned char sjwei = (in_the_block_pos-1)&0x1f | 0xa0;

    if (in_the_block_pos==31){      //31个特殊处理就行
        sjwei = 0xbf;
    }
    qDebug()<<"qswei......"<<qswei<<"......sjwei"<<sjwei;
    QByteArray ba;
    ba.resize(2);
    ba[0] = qswei;
    ba[1] = sjwei;
    uint16_t wcrc = ModbusCRC16(ba);
    QByteArray resba;
    qDebug()<<"CRC码：低字节:"<<uint8_t(wcrc)<<" 高字节:"<<uint8_t(wcrc>>8);
    resba.resize(4);
    unsigned char di=uint8_t(wcrc);
    unsigned char gao=uint8_t(wcrc>>8);
    resba[0]=qswei;
    resba[1] = sjwei;
    resba[2] = di;
    resba[3] = gao;
    emit sendMessageToseriesport(resba);
}


void SegmentCircuitDiagram::emitbacktomainsignal(){
    emit backtomain();
}


SegmentCircuitDiagram
::~SegmentCircuitDiagram
()
{
    delete ui;
}
