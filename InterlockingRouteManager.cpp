#include "InterlockingRouteManager.h"
#include "DataAccessLayer.h"
#include <QDebug>
#include <QRegularExpression>

InterlockingRouteManager::InterlockingRouteManager(QObject *parent)
	: QObject(parent)
{
	
}

InterlockingRouteManager::~InterlockingRouteManager()
{

}


void InterlockingRouteManager::handleRouteReuqest() {
	auto whatroute = dataAccessLayer.getLatestRoute();
	qDebug() << "Latest Route..............:";
	for (const QString& item : whatroute) {
		qDebug() << item;
	}
	if (whatroute.length() == 2) {
		auto matchingroute = dataAccessLayer.findMatchingRoute(whatroute.first(), whatroute.last());
		qDebug() << "Matching Route is ..............:";
		for (const QString& item : matchingroute) {
			qDebug() << item;
		}
		//判断当前有效的进路中，是否存在进路安排冲突
		auto requestpath = matchingroute.first();
		auto sections = matchingroute.last();
		auto hasconflick = isRouteConflicting(requestpath);
		qDebug() << "想要办理的route和已经存在于联锁表中的进路是否存在冲突 ..............:"<< hasconflick << requestpath;
		//判断所要经过的区段中的每一个区段的占用情况
		  // 使用正则表达式匹配轨道区段名称
		QRegularExpression re("'(.*?)'");
		QRegularExpressionMatchIterator matchIterator = re.globalMatch(sections);

		bool tracksectionpass = true;
		while (matchIterator.hasNext()) {
			QRegularExpressionMatch match = matchIterator.next();
			QString trackSection = match.captured(1); // 获取匹配的轨道区段名称
			qDebug() << trackSection;
			auto tracksectionstatus = dataAccessLayer.getTrackSectionStatus(trackSection);
			if (tracksectionstatus.toLower() != "Unknown") {
				tracksectionpass = false;
				break;
			}
		}
		if (hasconflick == false && tracksectionpass == true) {
			emit routeRequestPassChecked(requestpath);
		}
		
	}
	//首先从数据库中寻找对应的那些可以匹配到的进路信息，如果没有这不进行处理
}
