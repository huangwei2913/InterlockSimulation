#pragma once

#include <QObject>
#include <QTimer>

class CommandStatusChecker  : public QObject
{
	Q_OBJECT

public:
    CommandStatusChecker(QObject* parent) : QObject(parent), m_checkInterval(1000), m_timeoutInterval(10000) {}
    ~CommandStatusChecker();

signals:
    void checkRecordStatus(QString routepath, int recordId);
    void recordStatusChecked(int recordId, bool isValid);
    void allRecordsValid();
    void somethingFailed();


public slots:

    void startCheckingRecords(QString routepath, int commandID) {
        this->route_path = routepath;
        this->commandid = commandID;
        m_timer = new QTimer(this);
        m_timer->start(m_checkInterval);
        connect(m_timer, &QTimer::timeout, this, &CommandStatusChecker::bbbbb);
        m_timeoutTimer = new QTimer(this);
        connect(m_timeoutTimer, &QTimer::timeout, this, &CommandStatusChecker::handleTimeout);
        m_timeoutTimer->start(m_timeoutInterval);
    }


    void handleRecordStatusChecked(QString routepath, int recordId, bool isValid) {
        if (isValid) {
                m_timer->stop();
                m_timeoutTimer->stop();
                emit allRecordsValid();  
        }
        else {
            m_timer->stop();
            m_timeoutTimer->stop();
            emit somethingFailed();
        }
    }

    //只是为了让每秒发出一个check请求
    void bbbbb() {
       emit checkRecordStatus(route_path, commandid);
    }


    void handleTimeout() {
        m_timer->stop();
        emit somethingFailed();
    }


private:
    QTimer* m_timer;
    QTimer* m_timeoutTimer;
    int m_checkInterval;
    int m_timeoutInterval;
    QString route_path;
    int commandid;

};
