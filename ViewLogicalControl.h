#pragma once

#include <QObject>
#include <QMap>
#include "TrackView.h"
#include <QColor>
#include "DataAccessLayer.h"
#include "PhyDeviceManager.h"

//存储所有视图中的转辙机，区段状态，管理类似于界面中道岔是否可以定反操等
//而这个类相当于trackview的伴随管理类，实现在场景中特定的行为，例如，如果硬件报告的转辙机状态发生变化了，界面中的对象也要对应变化
class ViewLogicalControl  : public QObject
{
	Q_OBJECT

public:
	ViewLogicalControl(QObject *parent);
    ViewLogicalControl(QObject* parent, TrackView* trackView) : QObject(parent), m_trackView(trackView)
    {
		consolidateSwitchs.insert("1", "3");
		consolidateSwitchs.insert("3", "1");
		consolidateSwitchs.insert("5", "7");
		consolidateSwitchs.insert("7", "5");

		this->switchlockedMap.insert("1", false);
		this->switchlockedMap.insert("2", false);
		this->switchlockedMap.insert("3", false);
		this->switchlockedMap.insert("5", false);
		this->switchlockedMap.insert("4", false);
		this->switchlockedMap.insert("7", false);
		this->switchlockedMap.insert("9", false);
		this->switchlockedMap.insert("11", false);
		Line2CorlorMap.clear();
    }

	~ViewLogicalControl();

	void drawPath(const QString& pathString, QColor color);
	void setFriend(PhyDeviceManager* zz) {
		this->friend_device_manager = zz;
	}

public slots:
	void handlerSwitchLockingRequet(QStringList);
	void handlerInroutePathReuest(QString routepath);
	void handlerSignalDrawRequestForInroutePath(QString routepath);
	void handlerSwitchDirectedRequest(QString switchID);		//将某个道岔设置为定向
	void handlerSwitchReverseRequest(QString);
	void handlerSwitchLostRequest(QString);

	void handlerInoutePathClearRequest(QString routepath,QStringList segments );	//移除某个routepath的白光带,参考白光带会经过的区段


	void handlerTurnoutStatusUpdatedEvent(QString turnoutid, QString status);	//当道岔变化的时候，界面上也要发生变化
	void handlerSignalColorUpdatedEvent(QString signalid, QString colorstatus);	//当信号机发生变化的时候，需要检查界面上的颜色和沙盘上的信号机颜色是否一致
	void handlerTrackSectionStatusUpdatedEvent(QString sectionid, QString sectionstatus);																			//如果一致的话，则通知联锁控制允许发码，如果不一致的话则通知联锁控制系统做进一步检查
	void controlTrackSectionColor(QString sectionid, QString sectionstatus);	//控制场景视图中的区段颜色变化

signals:
	void InRouteArranged(QString routepath);
	void checklightcolorForInroute(QString routepath, QStringList signallist, QStringList colorList);  //应该要显示什么样的颜色
	void notifySegmentClearEvent(QString segmentname);  //通知联锁控制器，区段是空闲的
	void notifyInrouteSegmentTaken(QString segmentname);	//通知联锁控制器，所办理的进路中某一区段占用
	void notifySegmentTakenEvent(QString segmentname);		//通知联锁控制器，有一个区段被占用
	void notifySignalStatusCheckandPossibleFama(QMap<QString,QString> sig2colormap);	//通知联锁控制器，信号灯的颜色是一致的，指导联锁控制器进行可能得发码
	void notifySignalStatusInconsistence(QString, QString);			//通知联锁控制器，信号灯的颜色不一致，在联锁控制器中会进行进一步处理
	void drawPathForTurnoutChange(QString lineName, QPointF switchPos, QPointF endPos, QColor lineColor);  //在界面中转辙机失去表示，定操反操的时候都要调用这个在trackview上绘制
	void drawTurnoutTextForTurnoutChange(QString turnoutid, QString textcolor);
	void changeTracksectionColorRequest(QString sectionid, QString textcolor);
	//void drawSectionInthatTheStatusChanged(QString sectionid, QString textcolor);
	void notifySectionStatusChange(QString sectionid, QString textcolor);


private:
	QMap<QString, QString> consolidateSwitchs;  //联动转辙机
	QMap<QString, bool>   switchlockedMap;		//在界面中转辙机锁定的状态，当为false的时候，对应是可以操作的，当为true的时候
	QMap<QString, QString> signalcolorMap;		//信号机当前状态，可以通过这个m_trackView获得
	QMap<QString, QString> sectioncolorstatusMap;		//信号机当前状态，可以通过这个m_trackView获得

	QMap<QString, QPointF> switchPositionMap;			//由于这些是经常需要的，所以要加入进来减少对数据库依赖
	
	QMap<QString, QPointF> signalPositionMap;			//由于这些是经常需要的，所以要加入进来减少对数据库依赖


	TrackView* m_trackView;
	QMap<QString, QColor> Line2CorlorMap;  //画线的时候,记录他们的颜色
	DataAccessLayer da;
	const qreal kSwitchLineLength = 10.0; // 线段长度
	void drawSwitchsForInroutePath(const QString& routePath, const QString& todirectionJson);
	void removeLinesFromSwitch(const QString& switchName);
	void changeTurnoutTextColor(const QString& routePath, const QString& todirectionJson);

	void obtainUpdatedSignalColorMap();
	QString findBestMatchedTrack(const QString& routePath);
	PhyDeviceManager* friend_device_manager;
	void reloadSectionColorMap();	//重新从Trackview获取区段颜色，并存储在segmentcolorstatusMap中
	void reloadSwitchPositionMap(); //重新装载位置
	void reloadSignalPositionMap(); //重新装载信号机位置

public:
	bool getSecneSwitchLockStatus(QString switchNumber);	//获取场景中道岔转辙机的锁定状态


private:
	void lightLogicForSwitchDrawn(QString lineName,QColor linecolor);		//要简化代码

};
