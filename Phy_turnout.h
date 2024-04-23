#ifndef PHY_TURNOUT_H
#define PHY_TURNOUT_H

#include <QtSql>
#include <QSqlQuery>
#include <QString>

class Phy_Turnout {
public:
    Phy_Turnout(QString id) : id_(id) {}
    QString getStatus() const {
        return status_;
    }

    void setStatus(QString status) {
        status_ = status;
    }

    void setTrackSectionId(QString trackSectionId) {
        this->trackSectionId_ = trackSectionId;
    }
    QString getTrackSectionId() const {
        return trackSectionId_;
    }

private:
    QString id_;
    QString status_;
    QString trackSectionId_;
};


#endif // PHY_TURNOUT_H
