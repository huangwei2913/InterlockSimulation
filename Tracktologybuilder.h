#ifndef TRACKTOLOGYBUILDER_H
#define TRACKTOLOGYBUILDER_H

#include <QObject>
#include <QMap>
#include <QVector>

class TracktologyBuilder:public QObject
{
    Q_OBJECT
public:
    TracktologyBuilder();
    void addItemToMap(QString &item_name,QString &item_location);
    //在这里加入其它关于拓扑图的东西

signals:
    //任何事件触发,
    //void requestRoutePath(QString trainNo, QString startx, QString endx);                    //列车请求线路,第一个参数是列车编号，第二个参数是列车请求线路的开始标识，第三个参数是列车请求线路的结束标识
    //void reserverRoutePath(QString startx, QString endx);                   //外部发送过来的，或者自身经过判断需要为列车保留的线路
    void addRouteRequest(QString startx, QString endx);

public slots:
    //默认的列车编号是和谐号:harmony
    void ReserverRoutePath(QString trainNo, QString startx, QString endx);    //判断是否可以保留改线路
    void clearRoutePath(QString trainNo);                                     //清除为某列车保留的线路
    void updatekeepSignalStatus(QByteArray ba,int classno);  //更新信号机状态
    void updatekeepSwitchStatus(QByteArray ba);     //更新道岔状态
    void updatekeepSegmentstatus(QByteArray ba,int classno);    //更新区段状态
    void updateouteSelctionInfoFrom(QString fromwhere);    //在界面上单击时，到底选择何种进路
    void updateouteSelctionInfoto(QString towhere);    //在界面上单击时，到底选择何种进路
    void updateshiwuxinhaojihedaochaStatus(QByteArray ba);        //更新shiwusignalswithcStatus实物信号机和道岔状态


public:
    int scene_width;     //整个场景的高度和宽度
    int scene_height;

    QMap<QString, QString> item_location_map;  //每一个按钮项目的字符串和位置
    //每当一个进路信息已经传递，这个里面就要变化
    //下面这个数据结
    QVector<QString> reseveredPath;                         //之前已经保留的进路
    QMap<QString, QString> reseveredPathForTheTrain;        //通常只能为一辆列车保留一个线路，key是列车编号，值是线路x>y表示从x->y
    QMap<QString, unsigned char> switchsStatus;             //所有转辙机状态都存储在这个表中
    QMap<QString, unsigned char> signalsStatus;             //所有信号机状态都存储在这个表中
    QMap<QString, unsigned char> segmentsStatus;             //所有区段状态都存储在这个表
    QMap<QString, unsigned char> shiwusignalswithcStatus;             //实物信号机道岔状态存储
    QMap<QString, QString> cijiaSignalStatus;              //所有次加信号机当前所对应的颜色都存储在这个map中，20210925
    QMap<QString, QPointF> criticalPointsinToplogy;         //在轨道拓扑中，比较重要的节点，用于指导TopScene绘制不同区域的线
    void addCriticalPointsInformation(QString pointName, QPointF loc);                    //支持外部调用
    void requestRouteHandler(QString item_name);                        //请求进路时候的处理器
    void cacelRouteStartpointHanler(QString item_name);                      //取消进路开始选择， 注意一条进路总是从开始到结束，只不过有一些特殊的引导进路有可能不同

    int  getSegmentStatus(QString segment_name);         //根据指定的区段名称的状态，1代表已经占用，0表示未占用，-1表示未知
    int  getSwitchStatus(QString switch_name);           //根据指定的道岔名称，寻找其状态。0表示定位，1代表反位，-1表示未知
    int  getSingalStatus(QString signal_name);           //根据指定信号机名称，寻找其状态。1表示绿灯，3表示红灯，2表示黄灯，0表示灭灯，-1表示未知灯颜色，或者是故障

    QString trainloc;                                    //列车位置，这个是不断变化的
    QMap<QString,int>  trackName2code;                   //轨道名称对应数字编号，在后面编程使用


private:
    void updateCijiaXinhaoji();


public:
      //代表本次想要进行什么的进路
      QString fromStation;                             //每次在topscene中点击某些按钮的时候，这个地方就会被记录下来，
      QString toStation;                                //每次在主界面中选择combox进路选择选项都被记录下来

      //每次进路设置完成之后，这两个都被设置成""状态
      QString judgeTrainInWhichSegementation();               //获取当前train在哪个位置，判断当前train在哪个轨道区段
};

#endif // TRACKTOLOGYBUILDER_H
