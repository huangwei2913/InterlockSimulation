#pragma once

#include <QObject>
#include "Phy_Signal.h"
#include <QMap>
#include "DataAccessLayer.h"
#include "Phy_TrackSection.h"
#include "Phy_turnout.h"


class ViewLogicalControl;
//始终保持数据是最新的
class PhyDeviceManager  : public QObject
{
	Q_OBJECT
friend class ViewLogicalControl;
	//对所有硬件设备进行管理
public:


	PhyDeviceManager::PhyDeviceManager(QObject* parent)
		: QObject(parent)
	{}

	~PhyDeviceManager();
	
	void getUpdatedStatusForSignals();
	void getUpdateStatusForSections();
	QString updateAndFetchTrackSectionStatus(const QString& trackSectionId);
	Phy_TrackSection* getTrackSectionBySignalIds(const QString& startSignalId, const QString& endSignalId);
	Phy_Signal* getLastedColorForSignal(QString signal);  //从数据库中获取经过更新的信号灯颜色


signals:
	void turnoutStatusChanged(QString turnoutid, QString status);
	void trackSignalColorStatusChanged(QString turnoutid, QString status);
	void trackSectionStatusChanged(QString turnoutid, QString status);

public slots:
	void handlertrackSectionUpdatedMessage(QStringList _t1, QStringList _t2);
	void handlerTurnoutStatusUpdatedMessage(QString id, QString status);
	void handlerTrackSingaleUpdatedMessage(QString id, QString colorstatus);

private:
	QMap<QString, Phy_Signal*> physignal2map;		 //物理信号机集合
	QMap<QString, Phy_TrackSection*> physectionmap; //硬件有关的区段信息
	QMap<QString, Phy_Turnout*> phyTurnoutMap; //硬件有关的区段信息
	DataAccessLayer da;

public:
	QString getPhyTurnoutStatus(QString turnoutId);
	QString getPhySignalColorStatusById(QString signalID);

};
