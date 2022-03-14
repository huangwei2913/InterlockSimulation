#ifndef RECEIVERTHREAD_H
#define RECEIVERTHREAD_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QSerialPort>
#include "Helper.h"
#include  <QThreadPool>
//接收串口通信的
class SegmentCircuitDiagram;
class ReceiverThread: public QThread
{
    Q_OBJECT
public:
    explicit ReceiverThread(QObject *parent = nullptr);
    ~ReceiverThread();
    //开始接收和做出响应，响应是可以更新的
    void startReceiver(const QString &portName, int waitTimeout);
    void sendMessageToSeriealPort(QByteArray ba);  //只向串口哦发送数据
    void currentPortChanged(const QString &text);
signals:
    void request(const QString &s);         //为了跟踪下位机发送过来的东西，
    void shouldupdatexinhaojidaochazhuanzeji(QByteArray ba);        //发出需要更新信号机道岔转辙机信号
    void refreshSementStausOfUI(QString segments);         //检查特定区段状态，并更新操作UI上的区段对应颜色
    void refreshSwitchStatusOfUI(QString switchs);          //检查道岔状态，并更新UI上的道岔对应的颜色，用颜色来区分定操或者反操
    void error(const QString &s);
    void timeout(const QString &s);
    //由于非实物信号机采集状态时，会根据类别进行处理，所以codenumber参数必须要附加上。
    void feishiwusignalstatusreceieved(QByteArray ba,int codenumber);       //非实物信号机状态已经成功接收
    void feishiwuswitchstatusreceived(QByteArray ba);                       //非实物专责及状态已从串口接收
    void guidaoquduancaijistatus(QByteArray ba,int classno);                //轨道区段采集状态已从串口接收
    void shiwudaochaxinhaojistatus(QByteArray ba);                          //实物信号机等的状态编码

public slots:
    //可以在这里增加槽以改变端口,命令退出，改变回复等
     void startSendMessage(QByteArray ba);  //向串口发送数据

private:
    void run() override;     //重载运行函数
    QString m_portName;      //
    QString m_response;
    int m_waitTimeout = 0;
    QMutex m_mutex;         //每当由其他线程改变要从串口响应的值，或者设置串口的端口号时，可以使用该锁定装置
    bool m_quit = false;
    bool currentPortNameChanged = false;   //最开始的时候表示端口没变化
    QString currentPortName;
    QSerialPort serial;
    //
    int getCountofValidMessage(QString & tobecheckedString);  //返回有效的消息
    QString  getSpecifiedLocationMessage(QString tobecheckedString, int locindex);
    QThreadPool pool;
    QByteArray lastba_b8;   //关于串口更新轨道区段1101G，X1LQG，4DG，1G，3-11G时，最后一次得到的数据
    QByteArray lastba_b9;   //关于串口更新轨道区段1107G，1105G，1103G时，最后一次得到的数据
    QByteArray lastba_b2;   //关于串口更新道岔状态的

    int b8_update_flag;     //0表示没有更新，1表示有更新
    int b9_update_flag;     //0表示没有更新，1表示有更新
    int b2_update_flag;     //0表示没有更新，1表示有更新

};

#endif // RECEIVERTHREAD_H
