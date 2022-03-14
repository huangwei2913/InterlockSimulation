#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include "ToplogyScene.h"
#include <QGraphicsProxyWidget>
#include "Buttonwidget.h"
#include "CustomItemGroup.h"
#include "Mysignal.h"
#include <QSerialPort>
#include "ReceiverThread.h"
#include "MyLineItem.h"
#include "Tracktologybuilder.h"
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QToolTip>
#include "Helper.h"
#include "CustomDialog.h"
#include <QThreadPool>
#include "concurrentqueue.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SegmentCircuitDiagram;



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setSerialPort(QString serialPortName);   //设置串口PortName
    void displayMainwindow();

signals:
     void trainRequestRouteSingal(QString trainNo, QString startx, QString endx);   //列车请求某条线路


public slots:
    void slottrianlgle(int point);
    void showRequest(const QString &s);        //显示串口发过来的东西
    void updatesceneandreal(QByteArray ba);    //更新场景中所有道岔转辙机状态，更新界面中对应信号机状态
    void checkSegmentstatusAndRedrawLine(QString segments);    //checked and redraw segment line if train is on that segment
    void checkSwitchstatusAndRedrawRepresentation(QString switchs); //检查并且重新绘制道岔状态

    void showSegmentBlockCirculDiagram();       //显示区间闭塞电路图，为了调出界面
    void SegmentBlockCirculDiagramBack();       //从闭塞电路图返回

     void updateReserverPathinthescene(QString from, QString to, QString color);       //更新场景中为列车进路保留的线段
     void updateswithoperationsinthescene(QString whichswitch,QString whichoperation); //模拟定和反向操作
     void sendDriverCommand(QByteArray ba);
     void comboxItemchange(const QString &text);
     void sendmaxucommand(QString maxu);   //向串口发送码序
     void popUpDiaglog();
     void slotsingalColorSpecfied(QString item_name,QString color);  //此槽函数接收从topscene发出的信号变化指令执行指定操作


private:
    Ui::MainWindow *ui;


protected:
    void paintEvent(QPaintEvent* event) override;


public:
    void manageStackedWidgets();            //允许在桌面上点按钮切换到故障设置界面
    void addGreenLinetoSwitch(QPointF &sp, ToplogyScene *_mtoplogyscene, QColor color);     //在指定道岔位置处加入绿色，实际上是对道岔的正反位操作
    void addTextTolocation(QString text, QPointF pos, QColor color);          //在某个指定位置加入文本
    void addIsolationToSingal(QPointF start, QPointF end, QColor color);      //在特定的地方加入脚手架,其实可以当成线段来添加的
    void addFunctionButtons(QSize size);                                        //在根据场景的大小，在最底部加入按钮菜单以控制信号机
    //下面的函数五个参数，分别表示要在哪个代理中添加按钮，第二个参数表示指定的按钮，
    //第三个参数表示按钮名称，第四五个表示要添加到场景中的位置左上和右下角
    void addButtonToScene(ButtonWidget *button,  QString button_name, qreal aleft, qreal top);  //在场景中增加按钮
    void addSingalToScene(Mysignal *mysingal,qreal aleft, qreal atop,QColor color,QString signalname);  //加入信号名称                        //
    MyLineItem* addLineToScene(QPointF start,QPointF end);       //在何处画何种颜色的线端
    MyLineItem* addLineToScene(QPointF start,qreal len, qreal track_len_ratio);       //在何处画何种颜色的线端
    void addGroupToscene(CustomItemGroup *mygroup, qreal ax, qreal ay, QString textinfo);    //包括设置组合框选中的文字
    void addFourGroupsForIndication();
    void placeMarker();                 //在轨道中放入拓扑
    void addCriticalIsolations();       //在关键地方加入绝缘节
    TracktologyBuilder* tracktopstore;            //要配合联锁系统的铁道拓扑存储器
    void setSceneToplogyStore();                   //设置存储网络拓扑中的按钮位置对象
    //设置故障，设置数据格式
    //起始位是标识作用，259个故障点，需要9个故障板（需要结合硬件电路图来看），第一块板e1, 每块板
    //当要设置某个故障点的时候，数据位第七位是1，第6位是0，第五位是1，
    //在一个电路图上，
    // 一个系统出现故障的情况下，下发故障，同时下发100个故障点，选了100个，确认之后电脑暂存，第一个故障点，单个下发。
    //对整个故障点全复位，系统是好的，每块板都返回一个数据，信号机点什么灯是硬件驱动板，联锁系统把指令下发给驱采版，驱动信号机转折和动作之后，每隔500毫秒
    //想了解设备状态，然后使用采集指令采集，例如，控制信号机第0位，数据是0表示有效的，才能现实对应的颜色
    void initalallDeviceSingalStatus();             //采集所有上下位通过串口发送过来的关于道岔、转辙机等的状态
    void connectButtonWidgetsToTrackToplogy();      //关联进路按钮到TracktologyBuilder的槽函数
    void bindRouteRequesttolockinglogic();             //
    void initalInterlockingSceneStatus();                          //原始图初始化

private:

    QStackedWidget* mStackedWidget;
    SegmentCircuitDiagram* msegmentcidigmwidget;  //用这个窗口电路图
    QGraphicsView *tracktoplodgyview; //在主窗口的某个中央widget中创建一个显示区域
    ToplogyScene *mtoplogyscene;
    QGraphicsProxyWidget *proxy;      //在场景中添加普通的Qwidget
    ButtonWidget *mybutton;           //在场景中添加一个buttonwidgt

    CustomItemGroup * mytestgroup;
    Mysignal *x_signal_B1;                  //在场景中固定位置
    Mysignal *x_signal_B2;

    int single_button_width;                //所有信号灯和按钮的宽度
    int single_button_height;               //所有信号灯和按钮的高度

    Mysignal *sx_signal_1107_a;                  //在场景中固定位置
    Mysignal *sx_signal_1107_b;                  //信号机a,b位置

    Mysignal *sx_signal_1105_a;                  //在场景中固定位置
    Mysignal *sx_signal_1105_b;                  //信号机a,b位置
    Mysignal *sx_signal_1103_a;                  //在场景中固定位置
    Mysignal *sx_signal_1103_b;                  //信号机a,b位置
    Mysignal *sx_signal_1101_a;                  //在场景中固定位置
    Mysignal *sx_signal_1101_b;                  //信号机在1101轨处的位置
    QGraphicsProxyWidget *proxy_sn_a1;          //在场景中添加普通的Qwidget
    QGraphicsProxyWidget *proxy_sn_a2;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_SN_A1;               //在场景中添加一个buttonwidgt
    ButtonWidget *mybutton_SN_A2;               //在场景中添加一个buttonwidgt

    Mysignal *sn_signal_b1;                  //在场景中SN处的表示灯固定位置
    Mysignal *sn_signal_b2;                  //信号机在1101轨处的位置


    Mysignal *xi_signal_b1;                  //在场景中XI处的表示灯固定位置
    Mysignal *xi_signal_b2;                  //信号机在1101轨处的位置


    QGraphicsProxyWidget *proxy_xi_a1;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_XI_A1;               //在场景中添加一个buttonwidgt

    //在X处增加三个按钮
    QGraphicsProxyWidget *proxy_x_a1;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_X_A1;               //在场景中添加一个buttonwidgt

    QGraphicsProxyWidget *proxy_x_a2;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_X_A2;               //在场景中添加一个buttonwidgt

    QGraphicsProxyWidget *proxy_x_a3;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_X_A3;               //在

    Mysignal *signal_1108_b1;                  //在场景1108表示灯固定位置
    Mysignal *signal_1108_b2;                  //信号机在1108轨处的位置

    Mysignal *signal_1106_b1;                  //在场景1108表示灯固定位置
    Mysignal *signal_1106_b2;                  //信号机在1108轨处的位置

    Mysignal *signal_1104_b1;                  //在场景1108表示灯固定位置
    Mysignal *signal_1104_b2;                  //信号机在1108轨处的位置

    Mysignal *signal_1102_b1;                  //在场景1108表示灯固定位置
    Mysignal *signal_1102_b2;                  //信号机在1108轨处的位置

    Mysignal *signal_s_b1;                  //在场景1108表示灯固定位置
    Mysignal *signal_s_b2;                  //信号机在1108轨处的位置

    QGraphicsProxyWidget *proxy_s_a1;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_s_A1;               //在场景中添加一个buttonwidgt

    QGraphicsProxyWidget *proxy_s_a2;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_s_A2;               //在场景中添加一个buttonwidgt

    QGraphicsProxyWidget *proxy_s_a3;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_s_A3;               //在场景中添加一个buttonwidgt

    Mysignal *signal_xii_b1;                  //在场景1108表示灯固定位置
    Mysignal *signal_xii_b2;                  //信号机在1108轨处的位置

    QGraphicsProxyWidget *proxy_xii_a1;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_xii_A1;               //在场景中添加一个buttonwidgt

    Mysignal *signal_sii_b1;                  //在场景1108表示灯固定位置
    Mysignal *signal_sii_b2;                  //信号机在1108轨处的位置

    QGraphicsProxyWidget *proxy_sii_a1;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_sii_A1;               //在场景中添加一个buttonwidgt

    Mysignal *signal_XN_b1;                  //在场景1108表示灯固定位置
    Mysignal *signal_XN_b2;                  //信号机在1108轨处的位置


    QGraphicsProxyWidget *proxy_xn_a1;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_xn_A1;               //在场景中添加一个buttonwidgt

    QGraphicsProxyWidget *proxy_xn_a2;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_xn_A2;               //在场景中添加一个buttonwidgt

    QGraphicsProxyWidget *proxy_xn_a3;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_xn_A3;               //在场景中添加一个buttonwidgt


    Mysignal *signal_X4_b1;                  //在场景1108表示灯固定位置
    Mysignal *signal_X4_b2;                  //信号机在1108轨处的位置

    QGraphicsProxyWidget *proxy_x4_a1;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_x4_A1;               //在场景中添加一个buttonwidgt

    Mysignal *signal_S4_b1;                  //在场景1108表示灯固定位置
    Mysignal *signal_S4_b2;                  //信号机在1108轨处的位置

    QGraphicsProxyWidget *proxy_s4_a1;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_s4_A1;               //在场景中添加一个buttonwidgt

    Mysignal * signal_SI_b1;
    Mysignal * signal_SI_b2;


    QGraphicsProxyWidget *proxy_si_a1;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_si_A1;               //在场景中添加一个buttonwidgt

    Mysignal * signal_X3_b1;
    Mysignal * signal_X3_b2;

    QGraphicsProxyWidget *proxy_X3_a1;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_x3_A1;               //在场景中添加一个buttonwidgt

    Mysignal * signal_S3_b1;
    Mysignal * signal_S3_b2;

    QGraphicsProxyWidget *proxy_S3_a1;          //在场景中添加普通的Qwidget
    ButtonWidget *mybutton_S3_A1;

    //QSerialPort serialPort;                     //
    QString serialPortName;                     //默认的串口通信句柄COM1                                       //在区段信号灯上加入绝缘节
    ReceiverThread* m_thread_xiaweiji;          //用于与下位机进行串口通信线程
    int m_transactionCount = 0;                 //这个没用到

    qreal  outtracklen;                        //外环轨道总长度
    qreal  innertracklen;                      //内环轨道总长度
    qreal  outterradiance;                        //内部轨道半径
    qreal  innerradiance;                      //外部轨道半径
    qreal  width_ration_for_track;             //依据场景的宽度，和该变量表示的期望绘制区域绘制轨道的
    qreal  pi;                                 //圆周率
    qreal  totallen;                           //整个直线长度
    qreal ax_startx;
    qreal ay_startx;
    qreal XI_SN_len ;                    //XI到SN的距离是600米
    qreal SI_XI_len ;                    //从SI到XI的距离是400米
    qreal X_SI_len ;
    void  customSegments();                    //对有关区段信息初始化
    void  initialScene();                      //配置场景大小，位置信息
    void  configureOutterTrack();              //配置外圈轨道
    void  configureinnerTrack();                //配置内圈轨道
    void  configurXiaweijicomunication();       //配置与下位机通信模块
    QSize  scene_size;                          //场景大小
    qreal heightsequal;                         //把场景的高度等分成多少分
    qreal track_len_ratio;
    QPointF endSI;
    QPointF endXI;
    QPointF endSN;
    QPointF end1101;
    QPointF end1103;
    QPointF end1105;
    QPointF end1107;
    QPointF  sxcg_single_s3;
    qreal SN_1101_len;
    qreal _1103_1105_len;
    qreal _1101_1103_len;
    qreal _1105_1107_len;
    qreal length_1107_part1;
    qreal ax_xn;                       //内圈轨道的ax
    qreal ay_xn;                       //内圈轨道的ay
    QPointF endXII;
    QPointF end1102;
    QPointF end1104;
    QPointF end1106;
    QPointF end1108;
    QPointF startxn;
    QPointF endSII;
    QPointF endS;
    QPointF endX3;
    qreal S_1102_len;
    qreal _1102_1104_len;
    qreal _1104_1106_len;
    qreal _1106_1108_len;
    qreal dist_X_5=350;                         //信号灯X到道岔5的距离
    qreal dist_x_3 = dist_X_5+450;              //信号灯X到道岔3的距离
    qreal dist_x_11 = dist_X_5+450+300;         //信号灯X到道岔11的距离
    QColor c1{185,187,25};                      //绝缘节的颜色
    void associateTopscnewithMainwindows();     //连接主窗口与topscnene间的关联事件
    CustomDialog* dialog_x_gaifang;             //改方窗口要弹出的内容

 private:

    bool communicationFaMa(QString fama);                //对通信板发码，每次8个一起发送,成功返回 0x29 0xf5
    void addHiddenRouterEntrenceButton();                //增加隐藏进路确认按钮
    QMap<QString, QRectF> signal2rect;                 //区段对应低频编号
    void SendByteArrayTodevice(QByteArray ba);

};
#endif // MAINWINDOW_H
