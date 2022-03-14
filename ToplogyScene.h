#ifndef TOPLOGYSCENE_H
#define TOPLOGYSCENE_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include "Tracktologybuilder.h"
#include <QList>
#include <QStack>
#include <QVector>
#include "Helper.h"
#include "CLinkList.h"
#include <QQueue>
//用该场景来管理多个项目
#include "concurrentqueue.h"
#include <QThread>
#include <QStateMachine>
#include <QPropertyAnimation>
#include <QEventTransition>

//在ToplogyScene中增加不断运行的线程

class ToplogyScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit ToplogyScene(QObject *parent = 0);
    ~ToplogyScene();

signals:
    void buttonTargetName(QString button_name);               //所有button对象都绑定该信号
    void buttonTargetcalcelselect(QString button_name);
    void signalShouldDisplayStatus(QString item_name, QString color);  //表示某种灯应该显示何种颜色
    void shutdownTrainsignal(QString item_name);       //这些buttonwidget停止闪烁就是灭灯的意思
    void drawlineSignal(QString from, QString to, QString color);   //通知从哪里到哪里画什么线的信号
    void swithoperationsignaldisplay(QString whichswitch,QString whichoperation);                 //这个是在界面上操作的，道岔操作,也就是把某个道岔，例如11的一小段部分弄成黑色，表示道岔已经操作了
    void driverSwitchOperatin(QString whichswitch,QString whichoperation); //向串口发出命令消息，控制道岔活动，这个是控制硬件的

    void allowRouteSignal(QString startx, QString endx);     //准许进路信号，然后转到发码和控制阶段
    void stopButtonFlashSignal(QString button_name);
    void emitDriverCommand(QByteArray ba);        //从topscene中向串口发送消息
    void emitmaxubiao(QString maxu);              //根据列车位置，发送码序表

public slots:
    void requestRouteHandler(QString item_name);                        //请求进路时候的处理器
    void  timeoutjob();                                                 //这里也有一个timeout处理
    void cacelrequeRouteHanler(QString item_name);
    void tryinterlockingAroute(QString startx,QString endx);
    void RouteTakenHandler(QString startx,QString endx);
    void TryRouterPermitAndDrwaScene(QString startx, QString endx);
protected:
    void timerEvent(QTimerEvent * event) override; //起始是无时不刻在检查这些信号机或者转辙机状态，然后更新这些状态，
    //并且我门可以预测一些隐含的信号机状态
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;


protected:

    bool event(QEvent *event) override;
    bool m_bPressed; //鼠标是否按下
    QPointF m_ItemPos; //鼠标选中的QGraphicsItem在场景中的位置
    QPointF m_Pressed; //鼠标按下的位置点
    QPointF m_Offset; //鼠标移动的偏移量
    QGraphicsItem *m_pItemSelected; //鼠标选中的QGraphicsItem
    QPoint          m_shiftOrg;                         // 鼠标点选item中位置相对于item原点的偏移量

private:
    std::shared_ptr<TracktologyBuilder> anchortracktoplogy;
    void DelaySpecifyTimePeriod(int millseconds);
    QStack<QString> buttonclickedvents;      //每次从该//我门设置两个有向链表，模拟正向和反向运行时候的信号机相对位置，为寻找次架信号机做准备
    void clearButtonClickedEventAndReserverPath(QString startx, QString endx);  //下面这些都是对应发码的
    QMap<QString, QString> signal2Color;                  //所有信号机对应的颜色，
    QMap<QString,QString>  segment2Stauts;                //所有区段对应的状态，要么是 "Taken" 要么是"NotTaken"
    QMap<QString,QString>  switch2Direction;              //所有道岔对应的方向，要么是"directed" 要么是"reversed"

    QString generateFaMaSerial(QString segment_name);               //
    CLinkList *signallist;                     //信号机对应的列表
    CLinkList *segmentlist;                    //分段对应的列表
    QMap<QString, QString> signal_prote_segement;        //所有信号机所防护区段
    int getHowManyAccessibleSegmentsAhead(QList<QString> signalss);              //获取前面有多少个空闲区段数
    QString getSignalBySegmentprotected(QString protected_segement);

public:
    void setToplogyBuiler(std::shared_ptr<TracktologyBuilder> &anchortracktoplogy);
    void driverSignalUpdate(QString signal_name, QString signal_color);            //每次更新界面上的信号机颜色时候，应该要记得驱动指定信号机到达何种状态
    void pushCommanItemToQueue(QByteArray ba);    //将需要的命令投递到命令队列，用一个专门的线程来处理，做一个缓冲
    void interlockingLogicProcess();             // 当得到区段更新消息时，更新场景中所有信号机的颜色，发送不同的发码信息，以及更新轨道上所有信号机状态
    void checkSegmentTakenStatus(QString segments);   //once the segement status are collected, this function will be called
    void checkSwitchsDirections(QString switchs);   //每次道岔方向状态更新，该函数就会被调用，主要是为了使界面上的表示一直

private:
    void commomOperation();     //相同操作
    QString the_last_clicked_button_name;   //最后一次被单击的按钮
    unsigned char sendingchar;
    int startupfaflag;             //首次发码指令
    QQueue<QByteArray> localcomandqueue;                     //本地线程
    QMap<unsigned char,QString> qudongzhilingmap;            //描述每一个驱动板上面对应的16位是撒
    QMap<int,unsigned char> bit2colorcolor;                  //主要用于
    QByteArray driver4daocha(QString dfc);                       //"DC" or "fc"
    QByteArray driver1101_X1command(QString whichsignal,QString whichoperation);        //驱动为绿L，绿黄LU,红H
    QByteArray driver13579112command(QString whichswitch,QString whichoperation);        // whichswitch={1-3-5-7-9-11-2},whichoperation={DC,FC}


    QByteArray driverXColorCommand(QString colorinfo);

    QByteArray driverXXNS1command(QString whichsignal,QString whichoperation);        // whichsignal={X/XN/S1},whichoperation={S1:{FB,U,H,L},XN:{YB,2U,H,L,U}, X:{YB,2U,H,L,U}}
    QByteArray driverS2S3S4X2command(QString whichsignal,QString whichoperation);     //whichsignal={XII,S4,S3,SII}

    QByteArray driverSNColoarComand(QString colorinfo);

    QByteArray driverSSNX3X4command(QString whichsignal,QString whichoperation);      //whichsignal={S/SN/X4/X3}
    QByteArray driver110311051107command(QString whichsignal,QString whichoperation);      //whichsignal={1103/1105/1107}
    QByteArray driver1102110411061108command(QString whichsignal,QString whichoperation);      //whichsignal={1103/1105/1107}


    QByteArray driver1107ColoarComand(QString colorinfo);
    QByteArray driver1105ColoarComand(QString colorinfo);
    QByteArray driver1103ColoarComand(QString colorinfo);





    unsigned char convertStrToByte(QString str);              //将字符串转换为255内字节
    QString getmaxuwhilesegmentistaken(QString segment_name); //当某个分段被占用时，依据占用情况生成所有码序
    QString getProtectedSignalAheadForSegment(QString segment_name);              //获取指定区段之前区段的保护信号机
    void sceneSignalinitialStatus();   //场景中信号灯最初的颜色
    void guidaoSignalInitalStatus();      //驱动轨道中信号灯最初颜色
    void guidaoSwitchIntialStatus();      //初始化时，将所有的转辙机都驱动为定位状态

    std::string appendCRC(QByteArray ba);
    QVector<QString> interlockinglogic_step1();  //update the signals if the protected segment is taken, please notice that both the signal of the scene and the real simulation bed should be updated
    QString interlockinglogic_step2();  //we should generate maxu and emit the maxu to the bed
    void interlockinglogic_step3(QString maxu,  QVector<QString> alreadychangedSigals);       //based on the second step results, change the color of the color of all signal exclduing the one used in the first step and emit driver command to the signal of the testbed
    void initialfamacomand();

    //需要另外一个线程来辅助判断？？？？


};

#endif // TOPLOGYSCENE_H
