#ifndef PHY_SIGNAL_H
#define PHY_SIGNAL_H
#include <QtSql>
#include <QSqlQuery>
#include <QString>


class Phy_Signal {
public:
public:
    Phy_Signal() {
        this->light1_ = "";
        this->light2_ = "";
    }


    void setSignalID(QString ID) {
        this->id_ = ID;
    }
    
    QString getSignalID() {
        return this->id_;
    }

    QString getLight1() const {
        return light1_;
    }

    QString getLight2() const {
        return light2_;
    }

    void settLight1(QString light1)  {
        this->light1_ = light1;
    }

    void settLight2(QString light2) {
        this->light2_ = light2;
    }

    QString getColorStatus() {
        return colorstatus;
    }

    void settColorStatus(QString tmpcolors) {
        colorstatus = tmpcolors;
    }


private:
    QString id_;
    QString light1_;
    QString light2_;
    QString colorstatus;
};

#endif // PHY_SIGNAL_H
