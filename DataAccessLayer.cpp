#include "DataAccessLayer.h"


QString DataAccessLayer::getTrackSectionIdById(int id) {
    if (!db.open()) {
        qDebug() << "Error: " << db.lastError().text();
        return QString();
    }

    QSqlQuery query(db);
    query.prepare("SELECT trackSectionId FROM Phy_Turnout WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec()) {
        if (query.next()) {
            QString trackSectionId = query.value("trackSectionId").toString();
            db.close();
            return trackSectionId;
        }
        else {
            qDebug() << "No record found for id:" << id;
            db.close();
            return QString();
        }
    }
    else {
        qDebug() << "Error executing query:" << query.lastError().text();
        db.close();
        return QString();
    }
}


void DataAccessLayer::updateCommandTableSuccess(int commandId, bool success) {

    if (!db.open()) {
        qDebug() << "Error: " << db.lastError().text();
        return ;
    }
    QSqlQuery query(db);
    query.prepare("UPDATE command SET success = :success WHERE id = :commandId");
    query.bindValue(":success", static_cast<int>(success));
    query.bindValue(":commandId", commandId);

    if (!query.exec())
    {
        db.close();
        qDebug() << "Failed to update command success: " << query.lastError().text();
    }
    else
    {
        db.close();
        qDebug() << "Command success updated for ID: " << commandId;
    }

} //表示任务已成功执行


QPointF DataAccessLayer::getTurnoutPosition(const QString& id) {
    if (!db.open()) {
        qDebug() << "Error: " << db.lastError().text();
        return QPointF();
    }

    QSqlQuery query(db);
    query.prepare("SELECT x, y FROM View_Turnout WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        db.close();
        qDebug() << "Failed to get turnout position: " << query.lastError().text();
        return QPointF();
    }

    QPointF position;
    if (query.next()) {
        position.setX(query.value(0).toDouble());
        position.setY(query.value(1).toDouble());
    }

    db.close();
    return position;
}


void DataAccessLayer::updateTurnoutColor(const QString& id, const QString& color) {
    if (!db.open()) {
        qDebug() << "Error: " << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    db.transaction(); // 开始事务

    query.prepare("UPDATE View_Turnout SET color = :color WHERE id = :id");
    query.bindValue(":color", color);
    query.bindValue(":id", id);

    if (!query.exec()) {
        db.rollback(); // 事务回滚
        db.close();
        qDebug() << "Failed to update turnout color: " << query.lastError().text();
    }
    else {
        db.commit(); // 提交事务
        db.close();
        qDebug() << "Turnout color updated for ID: " << id;
    }
}


void DataAccessLayer::insertViewTurnoutTable(const QString& id, double x, double y, const QString& color) {
    if (!db.open()) {
        qDebug() << "Error: " << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO View_Turnout (id, x, y, color) VALUES (:id, :x, :y, :color)");
    query.bindValue(":id", id);
    query.bindValue(":x", x);
    query.bindValue(":y", y);
    query.bindValue(":color", color);

    if (!query.exec()) {
        db.close();
        qDebug() << "Failed to insert turnout: " << query.lastError().text();
    }
    else {
        db.close();
        qDebug() << "Turnout inserted with ID: " << id;
    }
}


QMap<QString, QPointF> DataAccessLayer::getAllTurnoutPositions() {
    QMap<QString, QPointF> turnoutPositions;

    if (!db.open()) {
        qDebug() << "Error: " << db.lastError().text();
        return turnoutPositions;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id, x, y FROM View_Turnout");

    if (!query.exec()) {
        db.close();
        qDebug() << "Failed to get all turnout positions: " << query.lastError().text();
        return turnoutPositions;
    }

    while (query.next()) {
        QString id = query.value(0).toString();
        double x = query.value(1).toDouble();
        double y = query.value(2).toDouble();
        turnoutPositions.insert(id, QPointF(x, y));
    }

    db.close();
    return turnoutPositions;
}
//视图中信号机的位置也是存储在数据库中
void DataAccessLayer::insertViewSignalTable(const QString& id, double x, double y) {
    if (!db.open()) {
        qDebug() << "Error: " << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO View_Signal (id, x, y) VALUES (:id, :x, :y)");
    query.bindValue(":id", id);
    query.bindValue(":x", x);
    query.bindValue(":y", y);

    if (!query.exec()) {
        db.close();
        qDebug() << "Failed to insert signal: " << query.lastError().text();
    }
    else {
        db.close();
        qDebug() << "Signal inserted with ID: " << id;
    }
}

QPointF  DataAccessLayer::getSignalPositionInView(const QString& id) {
    if (!db.open()) {
        qDebug() << "Error: " << db.lastError().text();
        return QPointF();
    }

    QSqlQuery query(db);
    query.prepare("SELECT x, y FROM View_Signal WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        db.close();
        qDebug() << "Failed to get signal position: " << query.lastError().text();
        return QPointF();
    }

    QPointF position;
    if (query.next()) {
        position.setX(query.value(0).toDouble());
        position.setY(query.value(1).toDouble());
    }
    db.close();
    return position;
}

QMap<QString, QPointF> DataAccessLayer::getAllSignalPositions() {
    QMap<QString, QPointF> signalPositions;

    if (!db.open()) {
        qDebug() << "Error: " << db.lastError().text();
        return signalPositions;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id, x, y FROM View_Signal");

    if (!query.exec()) {
        db.close();
        qDebug() << "Failed to get all signal positions: " << query.lastError().text();
        return signalPositions;
    }

    while (query.next()) {
        QString id = query.value(0).toString();
        double x = query.value(1).toDouble();
        double y = query.value(2).toDouble();
        signalPositions.insert(id, QPointF(x, y));
    }

    db.close();
    return signalPositions;
}

QPair<QString, QString> DataAccessLayer::getSignalLightColors(const QString& id) {
    if (!db.open()) {
        qDebug() << "Error: " << db.lastError().text();
        return QPair<QString, QString>();
    }

    QSqlQuery query(db);
    query.prepare("SELECT B1, B2 FROM View_Signal WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        db.close();
        qDebug() << "Failed to get signal light colors: " << query.lastError().text();
        return QPair<QString, QString>();
    }

    QString B1, B2;
    if (query.next()) {
        B1 = query.value(0).toString();
        B2 = query.value(1).toString();
    }

    db.close();
    return QPair<QString, QString>(B1, B2);
}

void DataAccessLayer::updateSignalLightColors(const QString& id, const QString& B1, const QString& B2) {
    if (!db.open()) {
        qDebug() << "Error: " << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    db.transaction(); // 开始事务

    query.prepare("UPDATE View_Signal SET B1 = :B1, B2 = :B2 WHERE id = :id");
    query.bindValue(":B1", B1);
    query.bindValue(":B2", B2);
    query.bindValue(":id", id);

    if (!query.exec()) {
        db.rollback(); // 事务回滚
        db.close();
        qDebug() << "Failed to update signal light colors: " << query.lastError().text();
    }
    else {
        db.commit(); // 提交事务
        db.close();
        qDebug() << "Signal light colors updated for ID: " << id;
    }
}


bool DataAccessLayer::insertViewTrackSectionData(const QString& id, const QString& status) {
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "Failed to open database:" << db.lastError().text();
            return false;
        }
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO View_TrackSection (id, status) VALUES (?, ?)");
    query.addBindValue(id);
    query.addBindValue(status);

    bool success = query.exec();
    db.close();
    return success;
}


bool DataAccessLayer::updateViewTrackSectionStatus(const QString& id, const QString& status) {
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "Failed to open database:" << db.lastError().text();
            return false;
        }
    }

    QSqlQuery query(db);
    bool success = false;

    db.transaction();
    try {
        query.prepare("UPDATE View_TrackSection SET status = ? WHERE id = ?");
        query.addBindValue(status);
        query.addBindValue(id);
        success = query.exec();
        if (!success) {
            throw QString("Failed to update track section status: %1").arg(query.lastError().text());
        }
        db.commit();
    }
    catch (const QString& error) {
        qDebug() << error;
        db.rollback();
        success = false;
    }

    db.close();
    return success;
}

QString DataAccessLayer::getViewTrackSectionStatus(const QString& id) {
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "Failed to open database:" << db.lastError().text();
            return QString();
        }
    }

    QSqlQuery query(db);
    query.prepare("SELECT status FROM View_TrackSection WHERE id = ?");
    query.addBindValue(id);
    query.exec();
    if (query.next()) {
        QString status = query.value(0).toString();
        db.close();
        return status;
    }

    db.close();
    return QString();
}


QMap<QString, QString> DataAccessLayer::getAllViewTrackSectionStatus() {
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "Failed to open database:" << db.lastError().text();
            return QMap<QString, QString>();
        }
    }

    QMap<QString, QString> statusMap;
    QSqlQuery query(db);
    query.exec("SELECT id, status FROM View_TrackSection");
    while (query.next()) {
        statusMap.insert(query.value(0).toString(), query.value(1).toString());
    }

    db.close();
    return statusMap;
}





