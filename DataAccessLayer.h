#ifndef DATAACCESSLAYER_H
#define DATAACCESSLAYER_H

#include <QObject>
#include <QtSql>
#include <QSqlQuery>
#include <QRandomGenerator>
#include "Phy_TrackSection.h"

class DataAccessLayer:public QObject
{
    Q_OBJECT
public:
    DataAccessLayer() {
        // 初始化数据库连接
        QString connectionName = "connection_" + QString::number(QRandomGenerator::global()->generate());
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        this->mconnectionName = connectionName;
        db.setDatabaseName("D:/myDb.db");
        if (!db.open()) {
            qDebug() << "Error: Failed to connect database." << db.lastError();
        } else {
            qDebug() << "Succeed to connect database with connection name:" << connectionName;
        }
    }

    ~DataAccessLayer() {
        // 关闭数据库连接
        db.close();
        QSqlDatabase::removeDatabase(mconnectionName);
    }




public:

    void populatePhyTrackSectionMap(QMap<QString, Phy_TrackSection*>& physectionmap) {
        if (db.open()) {
            QSqlQuery query(db);
            query.exec("SELECT * FROM Phy_TrackSection");

            while (query.next()) {
                QString id = query.value("id").toString();
                QString startSignalId = query.value("startSignalId").toString();
                QString endSignalId = query.value("endSignalId").toString();
                QString status = query.value("status").toString();

                Phy_TrackSection* section = new Phy_TrackSection();
                section->setTrackSectionId(id);
                section->setStartSignalId(startSignalId);
                section->setEndSignalId(endSignalId);
                section->setStatus(status);

                physectionmap.insert(id, section);
            }
            db.close();
        }
        else {
            qDebug() << "Unable to open database";
        }
    }



    QPair<QString, QString> queryPhySignalTable(const QString& id) {
        db.transaction(); // 开始事务
        QSqlQuery query(db);
        query.prepare("SELECT light1, light2 FROM Phy_Signal WHERE id = :id");
        query.bindValue(":id", id);
        if (query.exec() && query.next()) {
            QString light1 = query.value(0).toString();
            QString light2 = query.value(1).toString();
            db.commit(); // 提交事务
            return qMakePair(light1, light2);
        }
        else {
            db.rollback(); // 回滚事务
            qDebug() << "query error: " << query.lastError();
            return qMakePair(QString(), QString());
        }
    }






    void updateOrInsertSignalStatus(QString signalId, QString light1, QString light2) {
        db.open();
        QSqlQuery query(db);
        // 首先检查这个信号机的状态是否已经存在于表中
        query.prepare("SELECT COUNT(*) FROM Phy_Signal WHERE id = ?");
        query.addBindValue(signalId);
        query.exec();
        query.next();
        int count = query.value(0).toInt();

        if (count > 0) {
            // 如果这个信号机的状态已经存在于表中，那么就更新它的状态
            query.prepare("UPDATE Phy_Signal SET light1 = ?, light2 = ? WHERE id = ?");
            query.addBindValue(light1);
            query.addBindValue(light2);
            query.addBindValue(signalId);
            query.exec();
        } else {
            // 如果这个信号机的状态还不存在于表中，那么就插入新的数据
            query.prepare("INSERT INTO Phy_Signal (id, light1, light2) VALUES (?, ?, ?)");
            query.addBindValue(signalId);
            query.addBindValue(light1);
            query.addBindValue(light2);
            query.exec();
        }
        db.close();
    }

    QStringList getAllSignalIds()
    {
        QStringList signalIds;
        QSqlQuery query(db);

        if (query.exec("SELECT id FROM Phy_Signal"))
        {
            while (query.next())
            {
                signalIds.append(query.value("id").toString());
            }
        }
        else
        {
            qDebug() << "Error getting signal IDs: " << query.lastError().text();
        }

        return signalIds;
    }



    void updateTurnoutStatus(QString turnoutId, QString status) {
        // 检查数据库是否已经被打开，如果没有打开，那么尝试打开它
        if (!db.isOpen()) {
            if (!db.open()) {
                qDebug() << "Failed to open the database.";
                return;
            }
        }

        db.transaction();
        QSqlQuery query(db);

        // 直接更新转辙机的状态
        query.prepare("UPDATE Phy_Turnout SET status = ? WHERE id = ?");
        query.addBindValue(status);
        query.addBindValue(turnoutId);
        if (!query.exec()) {
            qDebug() << "Error updating turnout status:" << query.lastError().text();
            db.rollback();
            db.close();  // 关闭数据库连接
            return;
        }

        db.commit();
        db.close();  // 关闭数据库连接
    }



    //void updateTurnoutStatus(QString turnoutId, QString status) {
    //    db.transaction();
    //    QSqlQuery query(db);

    //    // 首先检查这个转辙机的状态是否已经存在于表中
    //    query.prepare("SELECT COUNT(*) FROM Phy_Turnout WHERE id = ?");
    //    query.addBindValue(turnoutId);
    //    if (!query.exec()) {
    //        qDebug() << "Error checking turnout status:" << query.lastError().text();
    //        db.rollback();
    //        return;
    //    }

    //    query.next();
    //    int count = query.value(0).toInt();
    //    qDebug() << "这里判断count是在干撒子？？？XXXXXXXXXXXXXXx" << count;
    //    if (count > 0) {
    //        // 如果这个转辙机的状态已经存在于表中，那么就更新它的状态
    //        query.prepare("UPDATE Phy_Turnout SET status = ? WHERE id = ?");
    //        query.addBindValue(status);
    //        query.addBindValue(turnoutId);
    //        if (!query.exec()) {
    //            qDebug() << "Error updating turnout status:" << query.lastError().text();
    //            db.rollback();
    //            return;
    //        }
    //    }

    //    db.commit();
    //}


    void updateTrackSectionStatus(QString trackSectionId, QString status) {
        db.open();
        QSqlQuery query(db);
        // 首先检查这个轨道段的状态是否已经存在于表中
        query.prepare("SELECT COUNT(*) FROM Phy_TrackSection WHERE id = ?");
        query.addBindValue(trackSectionId);
        query.exec();
        query.next();
        int count = query.value(0).toInt();

        if (count > 0) {
            // 如果这个轨道段的状态已经存在于表中，那么就更新它的状态
            query.prepare("UPDATE Phy_TrackSection SET  status = ? WHERE id = ?");
            query.addBindValue(status);
            query.addBindValue(trackSectionId);
            query.exec();
        }
        db.close();
    }


    QString getStartSignalId(const QString& trackSectionId) {
        db.open();
        QSqlQuery query(db);
        query.prepare("SELECT startSignalId FROM Phy_TrackSection WHERE id = ?");
        query.addBindValue(trackSectionId);
        qDebug() << "SQL query:" << query.lastQuery();
        qDebug() << "Bound parameter:" << trackSectionId;
        query.exec();
        if (query.next()) {
            auto tmp = query.value(0).toString();
            db.close();
            return tmp;
        }
        else {
            db.close();
            qDebug() << "Error getting route description:" << query.lastError().text();
            return QString();
        }
    }


    void printAllPhyTrackSectionRecords()
    {
        qDebug() << "this line is entered.............:" ;

        db.open();
        QSqlQuery query(db);
        query.exec("SELECT * FROM Phy_TrackSection");

        while (query.next())
        {
            QString id = query.value("id").toString();
            QString startSignalId = query.value("startSignalId").toString();
            QString endSignalId = query.value("endSignalId").toString();
            QString status = query.value("status").toString();

            qDebug() << "ID:" << id;
            qDebug() << "Start Signal ID:" << startSignalId;
            qDebug() << "End Signal ID:" << endSignalId;
            qDebug() << "Status:" << status;
            qDebug() << "--------------------";
        }

        db.close();
    }

    QString getEndSignalId(const QString& trackSectionId) {
        db.open();
        QSqlQuery query(db);
        query.prepare("SELECT endSignalId FROM Phy_TrackSection WHERE id = ?");
        query.addBindValue(trackSectionId);
        query.exec();
        if (query.next()) {
            auto tmp = query.value(0).toString();
            db.close();
            return tmp;
        }
        else {
            db.close();
            return QString();  // 返回一个空字符串，表示没有找到对应的信号机
        }
    }

    QStringList getAllValidRoutes() {
        db.open();
        QSqlQuery query(db);
        query.prepare("SELECT routePath FROM InterlockingRoute WHERE isValid = 1");
        query.exec();
        QStringList validRoutes;
        while (query.next()) {
            validRoutes.append(query.value(0).toString());
        }
        db.close();
        return validRoutes;
    }

    void updateRouteValidity(int routeNumber, bool isValid) {
        db.open();
        QSqlQuery query(db);
        query.prepare("UPDATE InterlockingRoute SET isValid = ? WHERE routeNumber = ?");
        query.addBindValue(isValid);
        query.addBindValue(routeNumber);
        query.exec();
        db.close();
    }

    void insertIntoInterlockingRoute(int routeNumber, const QString& routePath, const QDateTime& handlingTime, bool isValid, const QDateTime& cancellationTime) {
        db.open();
        QSqlQuery query(db);
        query.prepare("INSERT INTO InterlockingRoute (routeNumber, routePath, handlingTime, isValid, cancellationTime) VALUES (?, ?, ?, ?, ?)");
        query.addBindValue(routeNumber);
        query.addBindValue(routePath);
        query.addBindValue(handlingTime);
        query.addBindValue(isValid);
        query.addBindValue(cancellationTime);
        query.exec();
        db.close();
    }
    //确定进路到底要修改哪些转辙机，以及让转辙机处于什么状态
    bool updateRouteDescription(const QString& startSignalName, const QString& endSignalName, const QString& routeDescription)
    {
        db.open();
        QSqlQuery query(db);
        query.prepare("UPDATE RouteRecords "
            "SET routeDescription = :routeDescription "
            "WHERE startSignalName = :startSignalName AND endSignalName = :endSignalName");
        query.bindValue(":routeDescription", routeDescription);
        query.bindValue(":startSignalName", startSignalName);
        query.bindValue(":endSignalName", endSignalName);

        if (query.exec())
        {
            db.close();
            return true;
        }
        else
        {
            qDebug() << "Error updating route description:" << query.lastError().text();
            db.close();

            return false;
        }
    }

    QString getRouteDescription(const QString& startSignalName, const QString& endSignalName)
    {
        db.open();
        QSqlQuery query(db);
        query.prepare("SELECT routeDescription "
            "FROM RouteRecords "
            "WHERE startSignalName = :startSignalName AND endSignalName = :endSignalName");
        query.bindValue(":startSignalName", startSignalName);
        query.bindValue(":endSignalName", endSignalName);

        if (query.exec())
        {
            if (query.next())
            {
                db.close();
                return query.value("routeDescription").toString();
            }
            else
            {
                db.close();
                return QString();
            }
        }
        else
        {
            db.close();
            qDebug() << "Error getting route description:" << query.lastError().text();
            return QString();
        }
    }




    //通过进路路径，找到它在数据库中的ID
    int getInterLockingRouteIDByRoutePath(const QString& routePath) {
        db.open();
        QSqlQuery query(db);
        query.prepare("SELECT routeNumber FROM InterlockingRoute WHERE routePath = ?");
        query.addBindValue(routePath);
        query.exec();
        if (query.next()) {
            auto tmp = query.value(0).toInt();
            db.close();
            return tmp;
        }
        else {
            db.close();
            return -1;  // 返回-1，表示没有找到对应的进路编号
        }
    }


    void recordRoute(const QString& startSignal, const QString& endSignal) {
        db.open();
        QSqlQuery query(db);

        // 先检查数据库中是否已经存在相同的记录
            // 插入开始信号机名称和结束信号机名称
            query.prepare("INSERT INTO RouteRecords (startSignalName, endSignalName) VALUES (?, ?)");
            query.addBindValue(startSignal);
            query.addBindValue(endSignal);
            if (query.exec()) {
                qDebug() << "将要排列的进路信息插入到数据库中...." << startSignal << endSignal;
            }
            else {
                qDebug() << "插入数据失败:" << query.lastError().text();
            }
      
        db.close();
    }



    
    QStringList getLatestRoute() {
        QStringList latestRoute;

        if (!db.isOpen()) {
            if (!db.open()) {
                qDebug() << "无法打开数据库:" << db.lastError().text();
                return latestRoute;
            }
        }

        if (db.isOpen()) {
            QSqlQuery query(db);
            // 查询离当前时间最近的记录
            query.prepare("SELECT startSignalName, endSignalName "
                "FROM RouteRecords "
                "ORDER BY recordTime DESC "
                "LIMIT 1");

            if (query.exec()) {
                if (query.next()) {
                    latestRoute << query.value(0).toString(); // 获取最近记录的startSignalName
                    latestRoute << query.value(1).toString(); // 获取最近记录的endSignalName
                    qDebug() << "最新路径记录:" << query.value(0).toString() << query.value(1).toString();
                }
                else {
                    qDebug() << "没有找到最新的路径记录";
                }
            }
            else {
                qDebug() << "查询失败:" << query.lastError().text();
            }
        }
        else {
            qDebug() << "数据库未打开";
        }
        db.close();
        return latestRoute;
    }




    QStringList findMatchingRoute(const QString& startSignal, const QString& endSignal) {
        QStringList matchingRoute;

        QSqlQuery query(db);

        if (db.open()) {
            query.prepare("SELECT path, sections FROM Routes WHERE path LIKE ? AND path LIKE ?");
            query.addBindValue(QString("%1%").arg(startSignal));
            query.addBindValue(QString("%%2%").arg(endSignal));

            if (query.exec()) {
                while (query.next()) {
                    QString path = query.value(0).toString();
                    QString sections = query.value(1).toString();

                    // 检查路径是否完全匹配
                    if (path.startsWith(startSignal) && path.endsWith(endSignal)) {
                        matchingRoute << path;
                        matchingRoute << sections;
                        break; // 找到第一个匹配的就返回
                    }
                }
            }
        }

        db.close();
        return matchingRoute;
    }

    QString getTrackSectionStatus(const QString& trackSectionId) {
        QString status = "Unknown";
        QSqlQuery query(db);
        if (db.open()) {
            query.prepare("SELECT status FROM Phy_TrackSection WHERE id = ?");
            query.addBindValue(trackSectionId);

            if (query.exec()) {
                if (query.next()) {
                    status = query.value(0).toString();
                }
            }
        }
        db.close();
        return status;
    }


    std::pair<QString, QString> getStartEndSignals(const QString& trackSectionId) {
        std::pair<QString, QString> signalss = { "Unknown", "Unknown" };
        QSqlQuery query(db);

        if (db.open()) {
            query.prepare("SELECT startSignalId, endSignalId FROM Phy_TrackSection WHERE id = ?");
            query.addBindValue(trackSectionId);

            if (query.exec() && query.next()) {
                signalss.first = query.value(0).toString(); // 获取开始信号机
                signalss.second = query.value(1).toString(); // 获取结束信号机
            }
        }

        db.close();
        return signalss;
    }


    QString findSectionIdByStartAndEndSignal(const QString& startSignalId, const QString& endSignalId) {
        db.open();
        QSqlQuery query(db);
        query.prepare("SELECT id FROM Phy_TrackSection WHERE startSignalId = :startSignalId AND endSignalId = :endSignalId");
        query.bindValue(":startSignalId", startSignalId);
        query.bindValue(":endSignalId", endSignalId);
        if (query.exec() && query.next()) {
            db.close();
            return query.value(0).toString();
        }
        db.close();
        return QString("");
    }




    //获取轨道线路(下行线或者是上行线)的方向，要么是ForWard要么是Reverse
    QString getCurrentTrackDirectionStatus(const QString& direction) {
        QString currentStatus;
        db.open();
        QSqlQuery query(db);
        query.prepare("SELECT currentStatus FROM TrackDirection WHERE direction = :direction");
        query.bindValue(":direction", direction);
        qDebug() << "JJJJJJJJJJHHHHHHHHHHHHHHHHHHXXXX......................" << direction;
        if (query.exec()) {
            if (query.next()) {
                currentStatus = query.value(0).toString();
            }
        }
        else {
            qDebug() << "查询失败:" << query.lastError().text();
        }
        db.close();
        return currentStatus;
    }


    void insertCommand(const QByteArray& commandContent) {
        if (db.open()) {
            qDebug() << "start inser command.......... and open is ok....";

            QSqlQuery query(db);
            query.prepare("INSERT INTO command (content) VALUES (:content)");
            query.bindValue(":content", commandContent);

            if (query.exec()) {
                qDebug() << "命令插入成功";
                db.close();
            }
            else {
                db.close();
                qDebug() << "命令插入失败:" << query.lastError().text();
            }

        }
        else {
            qDebug() << "无法打开数据库:" << db.lastError().text();
            db.close();
        }
    }


    int insertSwitchOperationCommand(const QByteArray& commandContent) {
        int lastInsertId = -1;

        if (db.open()) {
            qDebug() << "start insert command.......... and open is ok....";

            QSqlQuery query(db);
            query.prepare("INSERT INTO command (content) VALUES (:content)");
            query.bindValue(":content", commandContent);

            if (query.exec()) {
                qDebug() << "命令插入成功";
                lastInsertId = query.lastInsertId().toInt();
            }
            else {
                qDebug() << "命令插入失败:" << query.lastError().text();
            }

            db.close();
        }
        else {
            qDebug() << "无法打开数据库:" << db.lastError().text();
            db.close();
        }

        return lastInsertId;
    }

    //主要是为了得到唯一ID
  
    int insertCommandWithoutContent() {
        db.transaction();
        QSqlQuery query(db);
        query.prepare("INSERT INTO command (success, retry) VALUES (?, ?)");
        query.addBindValue(1);
        query.addBindValue(0);

        if (query.exec()) {
            int newId = query.lastInsertId().toInt();
            db.commit();
            return newId;
        }
        else {
            db.rollback();
            qDebug() << "Error inserting command record:" << query.lastError().text();
            return -1;
        }
    }


    bool updateCommandRecord(int commandId, const QByteArray& content) {
        db.transaction();
        QSqlQuery query(db);
        query.prepare("UPDATE command SET success = 0, content = ? WHERE id = ?");
        query.addBindValue(content);
        query.addBindValue(commandId);

        if (query.exec()) {
            db.commit();
            return true;
        }
        else {
            db.rollback();
            qDebug() << "Error updating command record:" << query.lastError().text();
            return false;
        }
    }

    QString getTurnoutStatusById(QString id) {
        QString status;
        QSqlQuery query(db);
        if (db.open()) {
            query.prepare("SELECT status FROM Phy_Turnout WHERE id = :id");
            query.bindValue(":id", id);

            if (query.exec() && query.next()) {
                status = query.value(0).toString();
            }
            else {
                qDebug() << "查询失败:" << query.lastError().text();
            }

            db.close();
        }
        else {
            qDebug() << "无法打开数据库:" << db.lastError().text();
        }

        return status;
    }



    QList<QString> getUniqueTurnoutIdsByTrackSectionId(QString trackSectionId) {
        QList<QString> ids;
        QSqlQuery query(db);
        if (db.open()) {
            query.prepare("SELECT DISTINCT id FROM Phy_Turnout WHERE trackSectionId = :trackSectionId");
            query.bindValue(":trackSectionId", trackSectionId);

            if (query.exec()) {
                while (query.next()) {
                    ids.append(query.value(0).toString());
                }
            }
            else {
                qDebug() << "查询失败:" << query.lastError().text();
            }

            db.close();
        }
        else {
            qDebug() << "无法打开数据库:" << db.lastError().text();
        }

        return ids;
    }



    void updateTrackDescription(const QString& routePath, const QString& trackDescription)
    {
        QString startSignalName, endSignalName;
        QStringList pathElements = routePath.split(" -> ");
        startSignalName = pathElements.first().trimmed();
        endSignalName = pathElements.last().trimmed();
        db.open();
        QSqlQuery query(db);
        query.prepare("UPDATE RouteRecords SET startSignalName = :startSignalName, endSignalName = :endSignalName, trackDescription = :trackDescription WHERE routePath = :routePath");
        query.bindValue(":startSignalName", startSignalName);
        query.bindValue(":endSignalName", endSignalName);
        query.bindValue(":trackDescription", trackDescription);
        query.bindValue(":routePath", routePath);

        if (!query.exec())
        {
            qDebug() << "Error updating trackDescription: " << query.lastError().text();
        }

        db.close();
    }





    bool insertInterlockingRecord(const QString& routePath)
    {
        db.open();
        QSqlQuery query(db);
        query.prepare("INSERT INTO InterlockingRoute (routePath, handlingTime, isValid) "
            "VALUES (:routePath, :handlingTime, :isValid)");
        query.bindValue(":routePath", routePath);
        query.bindValue(":handlingTime", QDateTime::currentDateTime());
        query.bindValue(":isValid", true);

        if (query.exec())
        {
            db.close();
            return true;
        }
        else
        {
            qDebug() << "Error inserting route record:" << query.lastError().text();
            db.close();
            return false;
        }
    }



    int insertCommandAndGetId(QString route_path, const std::string& commandContent) {
        QSqlQuery query(db);
        if (!db.isOpen()) {
            db.open();
        
        }
        // 开始事务
        db.transaction();
        QString content = QString::fromStdString(commandContent);

        // 插入记录并获取自增ID
        query.prepare("INSERT INTO command (content) VALUES (:content)");
        query.bindValue(":content", content);
        if (query.exec()) {
            int newId = query.lastInsertId().toInt();

            // 提交事务
            db.commit();
            db.close();
            emit comandInserted(route_path, newId);
            return newId;
        }
        else {
            // 回滚事务
            db.close();
            db.rollback();
            return -1;
        }
    }


    //这个地方在查询
    bool iscomandRecordValid(int recordId)
    {
        if (db.isOpen() == false) {
            db.open();
        }
        QSqlQuery query(db);
        query.prepare("SELECT success FROM command WHERE id = :recordId");
        query.bindValue(":recordId", recordId);
        if (query.exec())
        {
            if (query.next())
            {
                int success = query.value(0).toInt();
                db.close();
                return success == 1;
            }
        }
        db.close();
        return false;
    }


    void updateOldestUnsuccessfulCommand(const QByteArray& content) {
        if (db.isOpen() == false) {
            db.open();
        }
        QSqlQuery query(db);
        query.prepare("UPDATE command "
            "SET success = 1 "
            "WHERE content = ? "
            "AND success = 0 "
            "ORDER BY time DESC "
            "LIMIT 1");
        query.addBindValue(content);
        if (!query.exec()) {
            qDebug() << "Error updating command table:" << query.lastError().text();
            db.close();
        }
        db.close();
    }

    QString getCommandContentById(int commandId)
    {
        if (!db.isOpen()) {
            db.open();
        }
        QSqlQuery query(db);
        query.prepare("SELECT content FROM command WHERE id = :id");
        query.bindValue(":id", commandId);

        if (query.exec())
        {
            if (query.next())
            {
                db.close();
                return query.value("content").toString();
            }
        }
        else
        {
            qDebug() << "Error getting command content: " << query.lastError().text();
        }
        db.close();
        return QString();
    }

    //通过id找到 TrackSection
    QString getTrackSectionIdById(int id);
    void updateCommandTableSuccess(int commandId, bool success); //表示任务已成功执行


public slots:
    void checkComandStatusById(QString route_path, int recordId) {
        // 查询数据库,检查记录状态是否变更为有效
        bool isValid = iscomandRecordValid(recordId);
        emit commandStatusChecked(route_path, recordId, isValid);
    }

signals:
    void comandInserted(QString route_path,int recordId); 
    void commandStatusChecked(QString route_path,int recordId, bool isValid);


public:
    QSqlDatabase db;
    QString mconnectionName;
    QPointF getTurnoutPosition(const QString& id);
    void  updateTurnoutColor(const QString& id, const QString& color);
    void  insertViewTurnoutTable(const QString& id, double x, double y, const QString& color);
    void insertViewSignalTable(const QString& id, double x, double y); //视图中信号机的位置也是存储在数据库中
    QPointF  getSignalPositionInView(const QString& id);
    QMap<QString, QPointF> getAllSignalPositions(); 
    QPair<QString, QString> getSignalLightColors(const QString& id);
    QMap<QString, QPointF> getAllTurnoutPositions(); 
    void updateSignalLightColors(const QString& id, const QString& B1, const QString& B2);
    bool insertViewTrackSectionData(const QString& id, const QString& status);
    bool updateViewTrackSectionStatus(const QString& id, const QString& status);
    QString getViewTrackSectionStatus(const QString& id);
    QMap<QString, QString> getAllViewTrackSectionStatus();

    void updateViewSignalB1(const QString& id, const QString& B1); //更新信号机的表示灯1，就是视图这个表
    void updateViewSignalB2(const QString& id, const QString& B2); //更新信号机的表示灯1，就是视图这个表



};

#endif // DATAACCESSLAYER_H
