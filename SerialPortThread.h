#ifndef SERIALPORTTHREAD_H
#define SERIALPORTTHREAD_H

#include <QThread>
#include <QtSerialPort/QSerialPort>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <QDebug>
#include <thread> // 添加头文件
#include "Utilty.h"
#include <QSqlError>


class SerialPortThread : public QThread {
    Q_OBJECT

public:
    SerialPortThread(const QString& portName, const QString& databasePath,QObject *parent = nullptr)
        :portName_(portName), databasePath_(databasePath),  QThread(parent) {

        this->connectionName_ = "myconnection";
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName_);
        db.setDatabaseName(databasePath_);
        if (!db.open()) {
            qDebug() << "Failed to open database";
        }

    }

    ~SerialPortThread() {
        db.close();
        QSqlDatabase::removeDatabase(connectionName_);
    }

    void SerialPortThread::run() override {
        while (true) {
            // 检查是否有其他线程正在访问数据库表
            bool isTableLocked = false;
            int retryCount = 0;
            const int maxRetries = 3;
            const int waitTime = 100; // 等待时间 (毫秒)

            while (retryCount < maxRetries) {
                QSqlQuery query(db);
                query.prepare("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='command' AND sql LIKE '%EXCLUSIVE%'");
                if (query.exec() && query.next() && query.value(0).toInt() > 0) {
                    isTableLocked = true;
                    break;
                }
                retryCount++;
                QThread::msleep(waitTime);
            }

            if (isTableLocked) {
                // 如果数据库表被锁定,等待一段时间后再尝试
                qDebug() << "Database table is locked, waiting...";
                QThread::msleep(waitTime * 10);
                continue;
            }

            // 执行数据库查询和串口写入操作
            QSqlQuery query(db);
            query.exec("SELECT id, content FROM command WHERE success = 0 AND retry < 3 ORDER BY time");
            while (query.next()) {
                QString id = query.value(0).toString();
                QByteArray command = query.value(1).toByteArray();
                qDebug() << "we going to write command.........BBBBBBBBBBb...." << command.toHex();
                emit writeRequest(command);
                QThread::msleep(100);
            }

            if (!query.next()) {
                // 在数据库中没有命令时发送数据到串口
                QByteArray tmpgallery;
                tmpgallery.resize(3);
                tmpgallery[0] = (char)sendingchar;
                tmpgallery[1] = (char)0x80;
                tmpgallery[2] = (char)0x8e;
                sendingchar = sendingchar + 1;
                qDebug() << "我们要发出一般的轮询指令...." << tmpgallery.toHex();
                emit writeRequest(tmpgallery);
                QThread::msleep(100);
                if (sendingchar > 0xcb) {
                    sendingchar = 0xc0;
                }
            }
        }

        db.close();
    }

private:
    QString portName_;
    QString databasePath_;
    QSqlDatabase db;
    QString connectionName_;





signals:
      void writeRequest(QByteArray);

private:
    unsigned char sendingchar = 0xc0;
};



#endif // SERIALPORTTHREAD_H
