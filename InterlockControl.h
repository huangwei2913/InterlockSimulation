#pragma once
#include <QObject>
#include "BlinkingSquare.h"
#include <QQueue>
#include <QDebug>
#include "DataAccessLayer.h"
#include "Phy_turnout.h"
#include "TrackView.h"
#include <QStateMachine>
#include <QState>
#include "CommandStatusChecker.h"
#include "PhyDeviceManager.h"
//这个类实现对轨道各对象的联锁控制
//存储所有物理转辙机状态
typedef QMap<QString, Phy_Turnout*> PhyTurnoutMap;
enum class RouteDirection { DownwardForward, DownwardReverse, UpwardForward, UpwardReverse, CrossLine }; //排列一个进路时，进路路径涵盖所有可能情况
enum class TrackDirection { Downward, Upward, Unknown };  //到底是下行线，上行线还是未知的
class InterlockControl : public QObject
{
    Q_OBJECT
public:
    //InterlockControl(QObject *parent);
    ~InterlockControl();
    InterlockControl(QObject* parent, TrackView* trackView);
public slots:
    void buttonClicked(QString buttonName);
    void operateTurnouts(QString routepath); //针对所要办理的进路，我们需要操作指定的道岔
    void checkSwitchLocation(QString, QString);		//检查道岔位置是否正确
    void handleRecordStatusChecked(QString routepath, int cmdID, bool isValid);
    void checkTrackSectionDirection(QString route_path); // 基于所订阅的方向来检查进路所涉及到的区段是否需要改变方向
    void checkSignalColorStatus(QString routepath, QStringList signallist, QStringList colorList, QMap<QString, int>whichIDs); //我们需要首先查询数据库中的记录是否已经成功执行，然后对比分析这些信号机的颜色请求是否满足要求
    void handlerSwitchDingcaoOperationRequest(QString switchName);
    void handlerSwitchReverseOperationRequest(QString swithcName);
    void handlerSegmentClearEvent(QString segmentName);
    void handlerSegmmentTakenEvent(QString segmentName);

public slots:
    void handleSwitchDirectOperationSuccess(QString switchId, QString commandId);
    void handleSwitchDirectOperationFailure(QString switchId, QString commandId);
    void handleSwitchReverseOperationSuccess(QString switchId, QString commandId);
    void handleSwitchReverseOperationFailure(QString switchId, QString commandId);
    void handlerDrivePhySignalStatusRequest(); //处理驱动物理信号机颜色变化
    void handlerSignalStatusCheckandPossibleFama(QMap<QString, QString>); //检查是否可以进行发码
signals:
    void differentButtonClicked();	//不同两个按钮被点击，期望排列进路
    void sameButtonClicked();		//相同两个按钮被点击，这个时候我们可以让这些信号机停止闪烁
    void notifyViewControllerLockSwitch(QStringList); //通知视图控制层锁定场景中的转辙机
    void notifyTrackCommanderSwitchOperation(QString, QString); //传递的是routePath以及要对应进行转辙机设置的
    void turnoutPassChecked(QString routepath);  //用户想要的进路已经通过道岔检查
    void sectionDirectionPassChecked(QString routepath); //用户想要的进路已经通过轨道区间改方操作检查
    void conductTrackFamaRequest(QString routepath);  //开始对轨道进行发码
    bool fieldValueMeetsRequirements(const QString& fieldValue);  //检查数据库中的值是否满足要求
    bool conversationLimitReached();			//检查与数据库的交互是否达到指定次数
    void needValidateTrackSectionDirection(QString routepath, QVector<QString> tracksectionNeedsChangeDirection); //到底哪些区段应该改变方向呢？
    void InterlockingFaMA(QString, QString, QString);
    void switchDirectOperationRequested(QString);  //联锁控制决定是否定操
    void switchReverseOperationRequested(QString);  //联锁控制决定是否反操
    void switchDirectOperationSucceeded();
    void switchDirectOperationFailed();
    void drawSwitchDirectedInScene(QString switchId);
    void drawSwitchReverseInScene(QString switchId);
    void drawSwitchLostInScene(QString switchId);

    void switchReverseOperationSucceeded();
    void switchReverseOperationFailed();
    void notifyTrackCommanderForDrivingSignalChange(QMap<QString, QString>);
    void notifyViewControllerSectionColorShoudChange(QString sgmentName, QString color);       //通知视图控制器
    void driverSignalColorInShapan(QString the_protectedSignal, QString color);

    void notifyViewControllerRemoveInroutePath(QString the_route_path_, QStringList segmentlist);  //通知视图控制器移除进路
        //通知视图控制器需要更加合理地控制保护机的颜色和状态，当它所保护的区段清除占用的时候
    void notifyViewControllerChangeSignalColorAndStatusCarefullyWhenSegmeneClear(QString the_protectedSignal, QString segmentName); 


private:
    int m_counter;
    QQueue<QString> m_queue;
    DataAccessLayer da;
    PhyTurnoutMap turnoutMap;
    TrackView* m_trackView;
    QMap<QString, QString> consolidateSwitchMap;  //联动转辙机
    PhyDeviceManager* m_phydevicemanager;   //查询信号机状态
    CommandStatusChecker* m_statusChecker;
    QStringList getForwardSignalsWithSpecificInRoute(QString inroute);
    void findAdjacentStrings(const QString& input, const QString& target, QString& prev, QString& next);
    double calculateAngleBetweenLineSegments(const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPointF& p4);
    RouteDirection getRouteDirection(const QString& routePath, const QMap<QString, QPointF>& switchPositionMap, const QMap<QString, QPointF>& signalPositionMap);
    QStringList getTrackSectionTakenByRoutepath(QString routepath);  //从数据库中获取指定进路所经过的轨道区段
    QStringList findSubPathsForCrossLineRoute(const QString& routePath);
    TrackDirection getLineDirectionFromPath(const QString& path);
    //假设我们知道进路在下行线运行，则依靠这个函数我们知道进路是正向还是反向
    RouteDirection getRouteDirectionForDownlinePath(const QString& path, const QMap<QString, QPointF>& switchPositionMap,
        const QMap<QString, QPointF>& signalPositionMap);
    RouteDirection getRouteDirectionForUplinePath(const QString& path, const QMap<QString, QPointF>& switchPositionMap,
        const QMap<QString, QPointF>& signalPositionMap);
public:
    void setFriend(PhyDeviceManager* zz) {
        this->m_phydevicemanager = zz;
    }
    void FaMaLogicControl(QString routePath);
private:
    void addedComplementaryFama(QString the_sg, QMap<QString, QString>& segment2fama);
    QStateMachine* m_stateMachine_switchDC;       //为道岔定操设计一个状态机
    QState* m_requestingState_DCSwitch;           //当收到定操请求时候，驱动状态机进入这个状态
    QState* m_checkingState_DCSwitch;              //
    QState* m_successState_DCSwitch;
    QStateMachine* m_stateMachine_switchFC;       //为道岔定操设计一个状态机
    QState* m_requestingState_FCSwitch;
    QState* m_checkingState_FCSwitch;              //
    QState* m_successState_FCSwitch;

private:
    void handleOutstationSegment(QString segmentName, QString lineDirection);

};