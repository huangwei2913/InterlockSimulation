#pragma once

#include <QObject>
#include <QMap>
#include "DataAccessLayer.h"
#include "PhyDeviceManager.h"

//这个类负责驱动设备到指定的状态，接受InterlockControl类的管理
class TrackComander  : public QObject
{
	Q_OBJECT

public:
	TrackComander(QObject *parent);
	~TrackComander();
	void setDeviceManager(PhyDeviceManager* phydevicemanger) {
		this->m_phydevicemanager = phydevicemanger;
	}

	void InitalPhyFaMa();	//初始化时候发码


public slots:
	void handlerSwitchOperationRequest(QString,QString);    //帮助检查道岔操纵，针对某个进路的
    void handlerTrackDirectionCheckRequet(QString, QVector<QString>);  //帮助检查所有区段的方向是否已经改变了，为了某个进路的
    void handlerSignalColorOperaion(QString routepath, QStringList signallist, QStringList colorList);
	void handlerFama(QString, QString, QString);
	void handlerSwitchDingcaoOperation(QString switchNumber);		//对某个转辙机进行定操操作
	void handlerSwitchReverseOperation(QString switchNumber);		//对某个转辙机进行反操操作
	void DrivePhySignalStatus(QMap<QString, QString> signal2color);

	void DriverSpecifiedSignalToSpecifiedColor(QString signalname, QString wanteclor);	


signals:
    void turnoutOperationComandSend(QString,QString);  //转辙机操作命令已下发
	void checkSignalColorStatusForRoute(QString routepath, QStringList signallist, QStringList colorList, QMap<QString,int>whichIDs); //我们需要首先查询数据库中的记录是否已经成功执行，然后对比分析这些信号机的颜色请求是否满足要求
	void newSwitchDirectCommandStored(QString switchId, QString commandId);  //在对道岔进行定操作的时候，由于不知道操作成功与否，
	void newSwitchReverseCommandStored(QString switchId, QString commandId);  //在对道岔进行反操作的时候，由于不知道操作成功与否，

private:
	
	QMap<QString, QString> turnoutInWhichRouteMap;  //在这个表中记录，道岔是输入下行线，还是上行线
	DataAccessLayer da;
	void driverOnePhysignalToSpecificColor(QString signal_name, QString wanted_color);
	PhyDeviceManager* m_phydevicemanager;
	bool communicationFaMa(QString fama, QString direction);
	void FaMaHandler(QString segments, QString famas, QString uplinkornot);

public:

	


};
