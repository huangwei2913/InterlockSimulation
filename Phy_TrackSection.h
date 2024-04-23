#ifndef PHY_TRACKSECTION_H
#define PHY_TRACKSECTION_H
#include <QtSql>
#include <QSqlQuery>
#include <QString>

class Phy_TrackSection {
public:
  
    Phy_TrackSection() {};
    ~Phy_TrackSection() {};

    QString getStatus() const {
        return status_;
    }


    void setStatus(QString status_)  {

        this->status_ = status_;
    }

    QString getStartSignalId() const {
        return startSignalId_;
    }
    
    void setTrackSectionId(QString trackID) {
        this->id_ = trackID;
        
    }

    void setStartSignalId(QString startSignalId_) {
        this->startSignalId_ = startSignalId_;
    }

    QString getEndSignalId() const {
        return endSignalId_;
    }

    void setEndSignalId(QString endSignalId_) {
        this->endSignalId_ = endSignalId_;
    }

private:
    QString id_;
    QString status_;
    QString startSignalId_;
    QString endSignalId_;
};
#endif // PHY_TRACKSECTION_H
