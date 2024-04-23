#ifndef PROJECTINITIALIZER_H
#define PROJECTINITIALIZER_H
#include <QtSql>
#include <QSqlQuery>

/*
 *在项目初始化时，要检查是否有数据库存在，如果数据库不存在，这创建之后，
 *创建对应的表，并对应地插入下面这些信息，
 *信号机包括：X-B1 X-B2 SI-B1 SI-B2 XI-B1 XI-B2 SN-B1 SN-B2
 * S-B1  S-B2 XII-B1 XII-B2  SII-B1 SII-B2 XN-B1 XN-B2 S3-B1 S3-B2 S4-B1
 * S4-B2 X4-B1 X4-B2 X3-B1 X3-B2  1101-B1 1101-B2 1102-B1 1102-B2 1103-B1
 * 1103-B2
 * 轨道区段包括  3-11DG （开始信号机为X， 结束信号机为SI）
 * X1LQG（开始信号机为SN，结束信号机为1101）
 *  1101G （开始信号机为1101，结束信号机为1103）
 *  1103G （（开始信号机为1103，结束信号机为1105）
 *  1105G （开始信号机为1105，结束信号机为1107）
 *   1107G （开始信号机为1107，结束信号机为X）
 *    2DG（开始信号机为S， 结束信号机为XII）
 *    IIG（开始信号机为XII，结束信号机为SII）
 *    1-9DG（开始信号机为SII，结束信号机为XN）
 *     S1LQG （（开始信号机为XN，结束信号机为1108）
 *     1108G（开始信号机为1108，结束信号机为1106）
 *     1106G（开始信号机为1106，结束信号机为1104）
 *     1104G（开始信号机为1104，结束信号机为1102）
 *     1102G（开始信号机为1102，结束信号机为S） 2DG （开始信号机为S，结束信号机为XII）4G(开始信号机为X4，结束信号机为S4) IG(开始信号机为SI，结束信号机为XI)  3G(开始信号机为S3，结束信号机为X3) 4DG(开始信号机为XI，结束信号机为SN),  还有8个转辙机（编号为：1,7, 9, 2, 5 , 3, 11, 4）
 * */
class ProjectInitializer {
public:
    ProjectInitializer(const QString& databasePath) : databasePath_(databasePath) {
        connectionName = QString::number(QRandomGenerator::global()->generate());
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(databasePath_);
        db.open();
    }

    ~ProjectInitializer() {
        db.close();
        QSqlDatabase::removeDatabase(connectionName);
    }

    void initializeDatabase() {
        createTables();
        insertTrackSectionData();
        insertTurnoutData();
        createOtherTables();
        initializeTrackDirectionTable();
    }

    void constructRouteTable();
     

    int getRecordCount() {
        if (db.open()) {
            QSqlQuery query;
            query.exec("SELECT COUNT(*) FROM Routes");
            if (query.next()) {
                int recordCount = query.value(0).toInt();
                return recordCount;
            }
        }
        return -1; // Return -1 if unable to retrieve record count
    }

private:
    void createTables() {
        createPhyTurnoutTable();
        createPhySignalTable();
        createPhyTrackSectionTable();
        createTrackDirectionTable();
        createViewTurnoutTable();
        createViewSignalTable();
        createViewTrackSectionTable();
    }

    void createPhyTurnoutTable() {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS Phy_Turnout ("
            "id INTEGER PRIMARY KEY, "
            "status TEXT CHECK(status IN ('Lost', 'Normal', 'Reverse')), "
            "trackSectionId TEXT"
            ")");
    }

    void createPhySignalTable() {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS Phy_Signal ("
            "id TEXT PRIMARY KEY, "
            "light1 TEXT CHECK(light1 IN ('Red', 'Yellow', 'Green', 'Black', 'White')), "
            "light2 TEXT CHECK(light2 IN ('Red', 'Yellow', 'Green', 'Black', 'White'))"
            ")");
    }

    void createPhyTrackSectionTable() {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS Phy_TrackSection ("
            "id TEXT PRIMARY KEY, "
            "startSignalId TEXT, "
            "endSignalId TEXT, "
            "status TEXT CHECK(status IN ('Unknown', 'Free', 'Occupied'))"
            ")");
    }

    void createViewTurnoutTable() {     //也是为了解决多线程问题，统一上面的
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS View_Turnout ("
            "id TEXT PRIMARY KEY, "
            "x REAL, "
            "y REAL, "
            "color TEXT CHECK(color IN ('Yellow', 'Red', 'Green'))"
            ")");
    }

    void createViewSignalTable() {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS View_Signal ("
            "id TEXT PRIMARY KEY, "
            "B1 TEXT, "
            "B2 TEXT, "
            "x REAL, "
            "y REAL"
            ")");
    }

    void createViewTrackSectionTable() {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS View_TrackSection ("
            "id TEXT PRIMARY KEY, "
            "status TEXT"
            ")");
    }



    void insertTrackSectionData() {
        QStringList trackSectionIds = { "3-11DG", "X1LQG", "1101G", "1103G", "1105G", "1107G", "2DG", "IIG", "1-9DG", "S1LQG", "1108G", "1106G", "1104G", "1102G", "2DG", "4G", "IG", "3G", "4DG" };
        QStringList startSignalIds = { "X", "SN", "1101", "1103", "1105", "1107", "S", "XII", "SII", "XN", "1108", "1106", "1104", "1102", "S", "X4", "SI", "S3", "XI" };
        QStringList endSignalIds = { "SI", "1101", "1103", "1105", "1107", "X", "XII", "SII", "XN", "1108", "1106", "1104", "1102", "S", "XII", "S4", "XI", "X3", "SN" };
        insertDataIntoPhyTrackSectionTable(trackSectionIds, startSignalIds, endSignalIds);
    }

    void insertDataIntoPhyTrackSectionTable(const QStringList& trackSectionIds, const QStringList& startSignalIds, const QStringList& endSignalIds) {
        QSqlQuery query(db);
        for (int i = 0; i < trackSectionIds.size(); ++i) {
            query.prepare("INSERT INTO Phy_TrackSection (id, startSignalId, endSignalId, status) SELECT ?, ?, ?, 'Unknown' WHERE NOT EXISTS (SELECT 1 FROM Phy_TrackSection WHERE id = ?)");
            query.addBindValue(trackSectionIds[i]);
            query.addBindValue(startSignalIds[i]);
            query.addBindValue(endSignalIds[i]);
            query.addBindValue(trackSectionIds[i]);
            query.exec();
        }
       // printAllPhyTrackSectionRecords();
    }

    void insertTurnoutData() {
        QMap<int, QString> turnoutTrackSections = { {5, "3-11DG"}, {3, "3-11DG"}, {11, "3-11DG"}, {4, "4DG"}, {1, "1-9DG"}, {7, "1-9DG"}, {9, "1-9DG"}, {2, "2DG"} };
        insertDataIntoPhyTurnoutTable(turnoutTrackSections);
    }

    void insertDataIntoPhyTurnoutTable(const QMap<int, QString>& turnoutTrackSections) {
        QSqlQuery query(db);
        for (auto it = turnoutTrackSections.begin(); it != turnoutTrackSections.end(); ++it) {
            query.prepare("INSERT INTO Phy_Turnout (id, status, trackSectionId) SELECT ?, 'Lost', ? WHERE NOT EXISTS (SELECT 1 FROM Phy_Turnout WHERE id = ?)");
            query.addBindValue(it.key());
            query.addBindValue(it.value());
            query.addBindValue(it.key());
            query.exec();
        }
    }

    void createOtherTables() {
        createCommandTable();
        createInterlockingRouteTable();
        createRouteRecordsTable();
    }

    void createCommandTable() {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS command ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "content TEXT, "
            "success INTEGER DEFAULT 0, "
            "time TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
            "retry INTEGER DEFAULT 0"
            ")");
    }

    void createInterlockingRouteTable() {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS InterlockingRoute ("
            "routeNumber INTEGER PRIMARY KEY, "
            "routePath TEXT, "
            "handlingTime TIMESTAMP, "
            "isValid BOOLEAN, "
            "cancellationTime TIMESTAMP"
            ")");
    }

    void createRouteRecordsTable() {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS RouteRecords ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "startSignalName TEXT, "
            "endSignalName TEXT, "
            "routeDescription TEXT, "
            "trackDescription TEXT, "
            "recordTime TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ")");
    }

    void createTrackDirectionTable() {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS TrackDirection ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "direction TEXT, "
            "currentStatus TEXT "
            ")");
    }


    void initializeTrackDirectionTable() {
        QSqlQuery query(db);

        // 检查 TrackDirection 表是否为空
        query.exec("SELECT COUNT(*) FROM TrackDirection");
        query.next();
        int count = query.value(0).toInt();

        // 如果表为空,则插入初始数据
        if (count == 0) {
            query.prepare("INSERT INTO TrackDirection (direction, currentStatus) VALUES (?, ?)");
            query.addBindValue("Up");
            query.addBindValue("Forward");
            query.exec();

            query.addBindValue("Down");
            query.addBindValue("Forward");
            query.exec();
        }
    }

    void printAllPhyTrackSectionRecords() {
        QSqlQuery query(db);
        query.exec("SELECT * FROM Phy_TrackSection");

        while (query.next()) {
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
    }
public:
    QString databasePath_;
    QSqlDatabase db;
    QString connectionName;
};

/**

X -> 5 -> 3 -> 11 -> S3
X -> 5 -> 7 -> 9 -> S4
X -> 5 -> 7 -> 9 -> SII -> XII -> 2 -> S
X -> 5 -> 3 -> 11 -> S3 -> X3 -> 4 -> SN
X -> 5 -> 3 -> 11 -> S3 -> X3
X -> 5 -> 3 -> 11 -> SI
X -> 5 -> 7 -> 9 -> S4 -> X4 -> 2 -> S
X -> 5 -> 7 -> 9 -> SII
X -> 5 -> 7 -> 9 -> S4 -> X4
X -> 5 -> 7 -> 9 -> SII -> XII
X -> 5 -> 3 -> 11 -> SI -> XI -> 4 -> SN
X -> 5 -> 3 -> 11 -> SI -> XI
SI -> 11 -> 3 -> 1 -> XN
SI -> XI -> 4 -> SN
SI -> 11 -> 3 -> 5 -> X
XI -> SI -> 11 -> 3 -> 1 -> XN
XI -> 4 -> SN
XI -> SI -> 11 -> 3 -> 5 -> X
SN -> 4 -> X3 -> S3 -> 11 -> 3 -> 5 -> X
SN -> 4 -> XI -> SI
SN -> 4 -> XI -> SI -> 11 -> 3 -> 5 -> X
SN -> 4 -> XI
SN -> 4 -> XI -> SI -> 11 -> 3 -> 1 -> XN
SN -> 4 -> X3 -> S3 -> 11 -> 3 -> 1 -> XN
SN -> 4 -> X3
SN -> 4 -> X3 -> S3
S -> 2 -> X4 -> S4 -> 9 -> 7 -> 1 -> XN
S -> 2 -> XII -> SII -> 9 -> 7 -> 1 -> XN
S -> 2 -> X4 -> S4 -> 9 -> 7 -> 5 -> X
S -> 2 -> XII -> SII -> 9 -> 7 -> 5 -> X
S -> 2 -> XII -> SII
S -> 2 -> X4 -> S4
S -> 2 -> X4
S -> 2 -> XII -> SII -> 9 -> S4
S -> 2 -> XII -> SII -> 9 -> S4 -> X4
S -> 2 -> XII
XII -> SII -> 9 -> 7 -> 5 -> X
XII -> SII -> 9 -> S4
XII -> 2 -> S
XII -> SII -> 9 -> S4 -> X4 -> 2 -> S
XII -> SII -> 9 -> 7 -> 1 -> XN
SII -> 9 -> 7 -> 5 -> X
SII -> 9 -> 7 -> 1 -> XN
SII -> 9 -> S4 -> X4
SII -> XII -> 2 -> S
SII -> 9 -> S4 -> X4 -> 2 -> S
XN -> 1 -> 7 -> 9 -> S4
XN -> 1 -> 3 -> 11 -> S3 -> X3 -> 4 -> SN
XN -> 1 -> 7 -> 9 -> S4 -> X4
XN -> 1 -> 7 -> 9 -> SII -> XII
XN -> 1 -> 3 -> 11 -> S3 -> X3
XN -> 1 -> 3 -> 11 -> S3
XN -> 1 -> 3 -> 11 -> SI
XN -> 1 -> 3 -> 11 -> SI -> XI
XN -> 1 -> 3 -> 11 -> SI -> XI -> 4 -> SN
XN -> 1 -> 7 -> 9 -> SII
XN -> 1 -> 7 -> 9 -> S4 -> X4 -> 2 -> S
XN -> 1 -> 7 -> 9 -> SII -> XII -> 2 -> S
S3 -> 11 -> 3 -> 5 -> X
S3 -> X3 -> 4 -> SN
S3 -> 11 -> 3 -> 1 -> XN
S4 -> 9 -> 7 -> 1 -> XN
S4 -> X4 -> 2 -> S
S4 -> 9 -> 7 -> 5 -> X
X4 -> S4 -> 9 -> 7 -> 1 -> XN
X4 -> 2 -> S
X4 -> S4 -> 9 -> 7 -> 5 -> X
X3 -> 4 -> SN
X3 -> S3 -> 11 -> 3 -> 5 -> X
X3 -> S3 -> 11 -> 3 -> 1 -> XN
 **/


#endif // PROJECTINITIALIZER_H
