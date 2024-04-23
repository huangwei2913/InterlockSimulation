#pragma once

#include <QObject>
#include "DataAccessLayer.h"

class InterlockingRouteManager  : public QObject
{
	Q_OBJECT

public:
	InterlockingRouteManager(QObject *parent);
	~InterlockingRouteManager();

    bool judgeWhetherOverlapped(QString from, QString against, int least_overlapped_elements) {
        QStringList from_elements = from.split(" -> ");
        QStringList against_elements = against.split(" -> ");

        if (least_overlapped_elements > from_elements.count() || least_overlapped_elements > against_elements.count()) {
            return false;
        }

        for (int i = 0; i < from_elements.count(); i++) {
            for (int j = i + least_overlapped_elements - 1; j < from_elements.count(); j++) {
                QStringList subpath_elements = from_elements.mid(i, j - i + 1);
                QString subpath = subpath_elements.join(" -> ");
                if (against.contains(subpath)) {
                    return true;
                }
            }
        }

        return false;
    }



    bool isRouteConflicting(const QString& route) {
        // 获取所有有效进路
        qDebug() << "进入到这里。。。。";
        QStringList validRoutes = dataAccessLayer.getAllValidRoutes();

        // 遍历所有有效进路
        for (const QString& validRoute : validRoutes) {
            // 如果给定的进路与任何有效进路有重叠，返回 true
            if (judgeWhetherOverlapped(route, validRoute, 2)) {
                return true;
            }
        }

        // 如果给定的进路与所有有效进路都没有重叠，返回 false
        return false;
    }






        // 如果没有找到重叠的子路径，那么就返回 false，表示没有冲突
 
public slots:
    void handleRouteReuqest();

signals:
    void routeRequestPassChecked(QString route_path);
    

private:
    DataAccessLayer dataAccessLayer;

};
