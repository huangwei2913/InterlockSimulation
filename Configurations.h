#pragma once

#include <QObject>
#include <QSet>

enum class SignalType {
	DownwardCXSignal,  // 下行侧线信号机
	UpwardCXSignal,    // 上行侧线信号机
	DownlineSignal,    // 下行信号机
	UplineSignal,      // 上行信号机
	Unknown            // 未知类型
};


class Configurations  : public QObject
{
	Q_OBJECT

public:
	Configurations(QObject *parent);
	~Configurations();


	static const QStringList outstationSignals;
	static const QStringList instationSignals;
	static const QStringList downlineSignals;
	static const QStringList uplineSignals;
	static const QStringList cxSignals;

	static const QStringList downlineTrackTraversing;
	static const QStringList uplineTraversing;
	static const QStringList downlineTrackTraversingCX ;
	static const QStringList uplineTraversingCX;

	static const QStringList instationSections;
	static const QStringList outstationSections;

	static const QStringList downwardCXSignals;	//下行侧线信号机

	static const QStringList upwardCXSignals;  //上行侧线信号机

	static const QStringList downwardTrackSections; //下行线会经过的轨道区段
	static const QStringList downwardCXTrackSections;//下行侧线会经过的轨道区段


	static const QStringList upwardTrackSections; //下行线会经过的轨道区段
	static const QStringList upwardCXTrackSections;//下行侧线会经过的轨道区段

	static const QStringList downlineOutstationSgments; //下行站外区段
	static const QStringList uplineOutstationSgments; //下行站外区段


	static SignalType determineSignalType(const QString& signalId) {
		if (Configurations::downwardCXSignals.contains(signalId)) {
			return SignalType::DownwardCXSignal;
		}
		else if (Configurations::upwardCXSignals.contains(signalId)) {
			return SignalType::UpwardCXSignal;
		}
		else if (Configurations::downlineTrackTraversing.contains(signalId)) {
			return SignalType::DownlineSignal;
		}
		else if (Configurations::uplineTraversing.contains(signalId)) {
			return SignalType::UplineSignal;
		}
		else {
			return SignalType::Unknown;
		}
	}


};



