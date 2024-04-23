#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "SerialPortThread.h"
#include "TracktologyBuilder.h"
#include "BlinkingSquare.h"
#include "SceneEclipse.h"
#include "sceneline.h"
#include "SceneText.h"
#include "TrackView.h"
#include "InterlockControl.h"
#include "InterlockingRouteManager.h"
#include "ViewLogicalControl.h"
#include "TrackComander.h"
#include "PhyDeviceManager.h"
#include "TracktologyBuilder.h"
#include "TracktologyBuilder.h"
#include "MenuPushButton.h"
#include "MenuPushButtonManager.h"
#include <QSerialPort>
#include "dataport.h"
#include "DataAccessLayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void bindserialEventsToTrackGraphStore();


public slots:
    void handleMenuButtonClicked(QString);
    void handleMenuButtonTimerTimeout();
    void handleBlinkSquareClicked(QString);
    void handleSwitchClicked(QString);

    void handleSerialReceivedData(const QByteArray& data);
    void handleSerialError(const QString& error);
    void handleSerialOpened();
    void handleSerialClosed();

    void handleSerialWriteRequet(const QByteArray& data);

    void transferturoutStatusToPhyDeviceManager(QString zz, QString mm);
    void transtrackSectionStatusToPhyDeviceManager(QStringList _t1, QStringList _t2); //只是作为信号传递使用


private:
    Ui::MainWindow *ui;
    SerialPortThread* m_thread_xiaweiji;          //用于与下位机进行串口通信线程
    TracktologyBuilder* tracktopstore;            //要配合联锁系统的铁道拓扑存储器
    BlinkingSquare *square;
    SceneEclipse* eclipse;
    SceneLine* line;
    SceneText *text;
    TrackView* trackview;
    QGraphicsScene* mtoplogyscene;
    InterlockControl* m_interlockcontrol;  //联锁控制主对象
    InterlockingRouteManager* routeManager; //进路控制管理
    ViewLogicalControl* viewlogicalController; //视图中相关对象管理，验证用户输入，禁止用户输入等
    TrackComander* m_trackComander;  //主要有用于发送道岔驱动等指令到硬件
    PhyDeviceManager* m_deviceManager;
    TracktologyBuilder* m_hardwareagent; //其实相当于一个硬件的代理

    QStateMachine machine;  //用状态机来管理联锁逻辑
    QState* startState;    //最开始等待按钮点击的状态
    QState* differentButtonState; //两个不同按钮点击进入的状态
    QState* sameButtonState;   //相同按钮被点击所进入的状态
    QState* turnoutCheckedState;  //进入到转辙机检查成功状态
    QFinalState* endState;
    QSize scen_size;
   


private:
    DataPort* myserialobj;
    void InitialSerialPortCommnication(QString databasePath_);   //开启与下位机通信通道，其实就是从数据库读，然后写消息到串口
    DataAccessLayer* da;

private:
    MenuPushButton* menuButton;
    MenuPushButtonManager* menuButtonManager;
    std::unique_ptr<MenuPushButton> menuButton1;
    std::unique_ptr<MenuPushButton> menuButton2;
    std::unique_ptr<MenuPushButton> menuButton3;
    std::unique_ptr<MenuPushButton> menuButton4;
    std::unique_ptr<MenuPushButton> menuButton5;
    std::unique_ptr<MenuPushButton> menuButton6;
    std::unique_ptr<MenuPushButton> menuButton7;
    std::unique_ptr<MenuPushButton> menuButton8;
    std::unique_ptr<MenuPushButton> menuButton9;

    QString menuButtonManagerName = "";       // 保存 menuButtonManager 对象的名称
    bool isButtonControlEnabled = false;
    QTimer menutimer;
    void InitialMenuButtonGroupAndArrangeTheirPosition();       //安排好位置


public:

public slots:
    void handleNewSwitchDirectCommand(QString switchId, QString commandId);   //处理TrackCommander的道岔执行定向操作请求
    void handleNewSwitchReverseCommand(QString switchId, QString commandId);   //处理TrackCommander的道岔执行反操作请求


signals:
    void requestSwitchDirectOperator(QString switchName);   //对某个道岔进行定位操作,联锁控制对象发出

    void requestSwitchReverseOperator(QString switchName);  //对某个道岔进行反位操作，联锁控制对象发出



    void switchDirectOperationSucceeded(QString switchId, QString commandId);
    void switchDirectOperationFailed(QString switchId, QString commandId);

    void switchReverseOperationSucceeded(QString switchId, QString commandId);
    void switchReverseOperationFailed(QString switchId, QString commandId);
    void turoutStatusTrasnferinng(QString turoutid, QString status);
    void trackSectionStatusTransfering(QStringList sections, QStringList status);


private:
    void bindSwitchOperationEvent();        //绑定道岔操作事件


public:

signals:
    void phySwithStatus(QByteArray ba);//实物转辙机4
    void PhySignalStatus(QByteArray ba); //实物信号机XI 1101
    void VirtualSwithStatus(QByteArray ba); //虚拟转辙机1,3,5,7,9,11,2
    void FackSignalstatus(QByteArray ba, int codenumber);
    void TrackSectionStatus(QByteArray ba, int codenumber);

     //这个函数通知联锁控制对沙盘信号机进行驱动
     void DrivePhySignalStatusRequest();

};
#endif // MAINWINDOW_H
