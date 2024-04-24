#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <QGraphicsView>
#include <QObject>
#include <QGraphicsView>
#include <QVector>
#include "sceneline.h"
#include <QPair>
#include "SceneEclipse.h"
#include "SceneText.h"
#include "BlinkingSquare.h"
#include "DataAccessLayer.h"
#include <QtConcurrent/qtconcurrentmap.h>



enum class SignalStatus { Open, Hidden, Closed };

class InterlockControl; // 前向声明
//其实这个view就相当于一个scene视图，其中哟
class TrackView : public QGraphicsView
{
    Q_OBJECT
 
 friend class InterlockControl; // 将InterlockControl声明为友元类
 friend class ViewLogicalControl;


public:
    TrackView(QGraphicsScene* scene, QWidget* parent = nullptr);
    ~TrackView(){}
    //在界面上负责画轨道区段
    void drawTrackSection();
    //在界面上负责画信号机
    void drawSignals();
    //在界面上画转辙机
    void drawSwitchs();
    void drawInrouteButton();
    void drawSignallables();
    void InitialSignalStatus();     //首次打开界面的时候，对界面中的信号机状态进行初始化
    void InitialSignalColor();      //首次打开界面的时候，需要对界面中的信号机颜色进行初始化
    //void InitalSignalBiaoshiDengColor(); //对信号机表示灯颜色进行初始化

public:
    QMap<QString, QPair<QPointF, QPointF>> linePositions;  // 用来存储每个线段(也就是每个轨道)的开始和结束位置(在视图中的逻辑位置)，主要用于绘制其它东西，确定其它界面上的例如圆圈，按钮等位置
   // QMap<QString,  QPointF> signalPositionMap;  // 用来存储每个信号机位置，我们会使用这个位置来进行正方向判断,假定信号机都是贴近在轨道上的,逻辑相对位置
   // QMap<QString, QPointF> switchPositionMap;  // 用来存储转辙机位置，我们会使用这个位置来进行正方向判断 这个转换成数据库中的View_Turnout表
    QMap<QString, SignalStatus> signalStatusMap;  // 用来存放信号机当前状态，到底是开放，关闭还是隐藏状态 open or close or hidden

    QMap<QString, SceneLine*> m_lineMap;                //用来存储每个区段对应的指针地址
    QMap<QString, SceneEclipse*> m_eclipseMap;          //用来存储每个表示灯对应的指针地址
    QMap<QString, QPointF> m_eclipseViewCodinates;        //用来存储每个表示灯在视图坐标系中的位置

    QMap<QString,SceneText*> m_textMap;                      //用来存储每个转辙机对应的指针地址、

    QMutex m_mutex_textMap;  // 用于保护m_textMap的互斥锁


    QMap<QString, BlinkingSquare*> m_blinksquareMap;        //用来存储每个按钮对应的指针地址、
    QMutex m_mutex_blinksquareMap;  // 用于保护m_blinksquareMap的互斥锁


public:
    QMap<QString, BlinkingSquare*> getBlinkSquareMap() {
        QMutexLocker locker(&m_mutex_blinksquareMap);  // 在访问m_blinksquareMap时锁定互斥锁
        return m_blinksquareMap;  // 返回m_blinksquareMap的复制
    }


    QMap<QString, SceneText*> getTextMap() {
        QMutexLocker locker(&m_mutex_textMap);  // 在访问m_textMap时锁定互斥锁
        return m_textMap;  // 返回m_textMap的复制
    }

public:
    void restoreTrackSectionStatus(const QString& name) //用来将某个指定的区段状态恢复到之前的状态
    {
        SceneLine* line = m_lineMap.value(name);  // 从 QMap 中获取 SceneLine* 指针
        if (line) {
            line->restore();  // 调用 SceneLine 对象的 restore 方法
        }
    }

    void restoreSignalStatus(const QString& name) //用来将某个指定的区段状态恢复到之前的状态
    {
        auto* line = m_eclipseMap.value(name);  // 从 QMap 中获取 SceneLine* 指针
        if (line) {
            line->restore();  // 调用 SceneLine 对象的 restore 方法
        }
    }



private:
    QGraphicsScene *scene;          //以下全都是为了画一些东西到界面上的
    DataAccessLayer dal;
    //QVector<SceneLine*> lines;  // 用来存储所有的 SceneLine 对象,所有视图都是基于视图的逻辑位置
    void addEclipsesToLines(const QString& lineName, bool above);
    void addEclipsesToLineEnds(const QString& lineName, bool startOrEnd, bool leftOrRight);
    void addEclipsesToLine(const QString& lineName, bool startOrEnd, bool leftOrRight, bool above);
    void addTextToLine(const QString& lineName, const QString& text, qreal positionRatio, bool above);
    void addBlinkingSquare(const QString& squareName, const QPointF &loc, const QColor& color, const qreal size); //设置名称，位置，颜色，大小
    void drawTextAtPosition(const QPointF& positionInView, const QString& text, const QColor& color, const QFont& font);

    QMap<QString, QGraphicsLineItem*> pathLines;  
    void drawPath(const QString& pathName, const QPointF& start, const QPointF& end, const QColor& color); //受视图逻辑控制层控制,只是
    void removePath(const QString& pathName);
    void drawPathBetweenSignals(const QString& fromSignal, const QString& toSignal, QColor color);
    //void drawSwitchsForInroutePath(QString routepath, QString todirectionJson);  //传递道岔位置
    //void drawSwitchsForInroutePath(const QString& routePath, const QString& todirectionJson);
   // const qreal kSwitchLineLength = 10.0; // 线段长度
    //专门为道岔操作定操和反操还有失去表示时候用到的函数
    void drawPathForSwitch(const QString& pathName, const QPointF& start, const QPointF& end, const QColor& color);
    void setSignalLightColorInScene(const QString& signalId, const QString& desiredColor);
    void addSignalLightsToLine(const QString& lineName);
    bool isSignalRed(const QString& signalId);
    void drawPathForSwitch_modified(const QString& pathName, const QPointF& start, const QPointF& end, const QColor& color);



public:    
    QString getSignalColorByName(QString signal_name);
    QMutex m_eclipseMapLock;

    QMutex switchPositionMapLock;

    void SetSignalStatusToClose(QString signalName);    //将信号机状态设置为关闭状态 
    void SetSignalStatusToOpend(QString signalName);    //将信号机状态设置为开放状态 

public slots:
    void handlerdrawPathForTurnoutChange(QString lineName, QPointF switchPos, QPointF endPos, QColor lineColor);
    void handlerdrawTurnoutTextForTurnoutChange(QString turnoutid, QString textcolor);
    void handlerchangeTracksectionColorRequest(QString sectionid, QString colorsate);
};


#endif // TRACKVIEW_H
