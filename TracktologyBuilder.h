#ifndef TRACKTOLOGYBUILDER_H
#define TRACKTOLOGYBUILDER_H

#include <QObject>
#include <QMap>
#include <QVector>
#include "DataAccessLayer.h"

 //这个类分析串口原始数据，并更新数据库中对应转辙机，信号机和轨道区段状态
//其实是个工具类而已，从原始数据提取重要的抽象状态信息而已
class TracktologyBuilder:public QObject
{
    Q_OBJECT
public:
    TracktologyBuilder();
    TracktologyBuilder::TracktologyBuilder(QObject* parent)
        : QObject(parent)
    {}

signals:
    void trackSectionUpdated(QStringList tracksection, QStringList status);
    void turoutStatusUpdated(QString turnoutid, QString status);
    void trackSignalStatusUpdated(QString turnoutid, QString status);


public slots:
    void updateFakeSignalStatus(QByteArray ba,int classno);  //更新虚假的信号机状态
    void updateRealSignalStatus(QByteArray ba);              //更新实物信号状态
    void updateFakeSwitchStatus(QByteArray ba);              //更新虚拟的转辙机状态
    void updateRealSwithStatus(QByteArray ba);
    void updatTrackSectionStatus(QByteArray ba,int classno);




private:
    DataAccessLayer da;
    QMap<QString, unsigned char> rawVirtualSignalsStatus;             //所有虚拟信号机状态都存储在这个表中
    QString InspectFakeSingalStausFromRawData(QString signal_name);  //我们从串口数据获得信号机状态，
    QMap<QString, unsigned char> rawPhySignalsStatus;             //所有信号机状态都存储在这个表中
    void getFakeSignalColors(QMap<QString, unsigned char>
                             &rawSignalsStatus, QString signal,
                             QString &color1, QString &color2);

    QMap<QString, unsigned char> rawVitualSwitchsStatus;             //所有转辙机状态都存储在这个表中
    QMap<QString, unsigned char> rawPhySwitchsStatus;             //所有转辙机状态都存储在这个表中
    QMap<QString, unsigned char> rawSegmentsStatus;             //所有区段状态都存储在这个表

    //'Lost', 'Normal', 'Reverse'
    QString InspectVitualSwitchStatusFromRawData(QString switchNum,  QMap<QString, unsigned char>& switchsStatus) {
        QString dbKey = switchNum + "|DB";
        QString fbKey = switchNum + "|FB";

        if (switchsStatus[dbKey] == 0x00 && switchsStatus[fbKey] == 0x01) {
            return "Normal";
        } else if (switchsStatus[dbKey] == 0x01
                   && switchsStatus[fbKey] == 0x00) {
            return "Reverse";
        } else {
            return "Lost";
        }
    }

    QString InspectSegmentStatusFromRawData(QString switchNum,
                                                 QMap<QString, unsigned char>&
                                            switchsStatus) ;



    QString inspectSegmentStatusFromRawData(QString segment_name, int FQJH, int FQJQ, int GJQ) {
        if(rawSegmentsStatus[segment_name+"|"+"FQJH"]==FQJH
                && rawSegmentsStatus[segment_name+"|"+"FQJQ"]==FQJQ
                && rawSegmentsStatus[segment_name+"|"+"GJQ"]==GJQ
                ){
            return "Occupied";
        }

        if(rawSegmentsStatus[segment_name+"|"+"FQJH"]==0x00 &&
                rawSegmentsStatus[segment_name+"|"+"FQJQ"]==0x01
                && rawSegmentsStatus[segment_name+"|"+"GJQ"]==0x00
                ){
            return "Free";
        }

        return "Unknown";
    }


    QString inspectSegmentStatusFromRawData(QString segment_name){

        if(rawSegmentsStatus[segment_name+"|"+"GFJH"]==0x00 &&
                rawSegmentsStatus[segment_name+"|"+"GFJQ"]==0x01 &&
                rawSegmentsStatus[segment_name+"|"+"GJQ"]==0x01){
            return "Occupied";
        }

        if(rawSegmentsStatus[segment_name+"|"+"GFJH"]==0x01 &&
                rawSegmentsStatus[segment_name+"|"+"GFJQ"]==0x00 &&
                rawSegmentsStatus[segment_name+"|"+"GJQ"]==0x01){
            return "Occupied";
        }


        if(rawSegmentsStatus[segment_name+"|"+"GFJH"]==0x00 &&
                rawSegmentsStatus[segment_name+"|"+"GFJQ"]==0x01 &&
                rawSegmentsStatus[segment_name+"|"+"GJQ"]==0x00){
            return "Free";
        }

        if(rawSegmentsStatus[segment_name+"|"+"GFJH"]==0x01 &&
                rawSegmentsStatus[segment_name+"|"+"GFJQ"]==0x00 &&
                rawSegmentsStatus[segment_name+"|"+"GJQ"]==0x00){
            return "Free";
        }
        return "Unknown";
    }







};

#endif // TRACKTOLOGYBUILDER_H
