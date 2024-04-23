#include "Configurations.h"

Configurations::Configurations(QObject *parent)
	: QObject(parent)
{}

Configurations::~Configurations()
{}

const QStringList Configurations::outstationSignals = { "1107" ,"1105" ,"1103","1101","1102" ,"1104" ,"1106","1108" };
const QStringList Configurations::instationSignals = { "X" ,"SI" ,"XI","SN" ,"S3" ,"X3" , "XN" ,"SII" ,"XII","S" ,"S4" ,"X4" };

const QStringList  Configurations::downlineSignals = { "X", "SI", "XI","SN", "S3", "X3"};
const QStringList  Configurations::uplineSignals = { "XN", "SII", "XII", "S","S4", "X4" };

const QStringList Configurations::cxSignals = { "S3","X3", "S4", "X4" };

const QStringList Configurations::downwardCXSignals = { "S3","X3" };		//下行侧线信号机
const QStringList Configurations::upwardCXSignals = { "S4","X4" };		//上行侧线信号机
const QStringList Configurations::downlineTrackTraversing = { "X", "SI" ,"XI" , "SN", "1101" , "1103" , "1105" , "1107" };
const QStringList Configurations::uplineTraversing = { "S", "XII" ,"SII" , "XN", "1108" , "1106" , "1104" , "1102" };

const QStringList Configurations::downlineTrackTraversingCX = { "X", "S3" ,"X3" , "SN", "1101" , "1103" , "1105" , "1107" };
const QStringList Configurations::uplineTraversingCX = { "S", "X4" ,"S4" , "XN", "1108" , "1106" , "1104" , "1102" };


const QStringList Configurations::downlineOutstationSgments = { "X1LQG","1101G","1103G","1105G","1107G" };
const QStringList Configurations::uplineOutstationSgments = { "S1LQG","1108G","1106G" ,"1104G" ,"1102G" };


//严格意义上SlLQG不是站内区段, 区分站内和站外区段
const QStringList Configurations::instationSections = {"3-11DG","IG","4DG","3G","1-9DG","IIG","4G","2DG"};
const QStringList Configurations::downwardTrackSections = { "3-11DG" ,"IG", "4DG","X1LQG","1101G","1103G","1105G","1107G" };
const QStringList Configurations::downwardCXTrackSections = { "3-11DG" ,"3G", "4DG","X1LQG","1101G","1103G","1105G","1107G" };

const QStringList Configurations::upwardTrackSections = {"2DG","IIG","1-9DG","S1LQG","1108G","1106G" ,"1104G" ,"1102G" }; //上行线会经过的轨道区段
const QStringList Configurations::upwardCXTrackSections = { "2DG","4G","1-9DG","S1LQG","1108G","1106G" ,"1104G" ,"1102G" };//上行侧线会经过的轨道区段



