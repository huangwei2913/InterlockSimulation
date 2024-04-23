#include "TrackComander.h"
#include <QDebug>
#include <QMap>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include "Utilty.h"


//这个类只是发送命令给硬件端
TrackComander::TrackComander(QObject *parent)
	: QObject(parent)
{
    turnoutInWhichRouteMap.insert("4", "Down");  //4号道岔在下行线
    turnoutInWhichRouteMap.insert("5", "Down");  //4号道岔在下行线
    turnoutInWhichRouteMap.insert("3", "Down");  //4号道岔在下行线
    turnoutInWhichRouteMap.insert("11", "Down");  //4号道岔在下行线

    turnoutInWhichRouteMap.insert("2", "Up");  //4号道岔在下行线
    turnoutInWhichRouteMap.insert("1", "Up");  //4号道岔在下行线
    turnoutInWhichRouteMap.insert("7", "Up");  //4号道岔在下行线
    turnoutInWhichRouteMap.insert("9", "Up");  //4号道岔在下行线
}

TrackComander::~TrackComander()
{}


void TrackComander::handlerSwitchOperationRequest(QString routePath,QString json) {
	qDebug() << "...handlerSwitchOperationRequest." << routePath << json;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject jsonObject = jsonDocument.object();

    // 将 QJsonObject 转换为 QMap
    QMap<QString, QString> tempMap;
    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
        tempMap.insert(it.key(), it.value().toString());
    }

    // 打印转换后的 QMap,只对实物道岔
    for (auto it = tempMap.begin(); it != tempMap.end(); ++it) {
        qDebug() << "Key:" << it.key() << "Value:" << it.value();
        
        if (it.key() == "4") {      //如果是第4号道岔，
                
            auto direction = turnoutInWhichRouteMap.value(it.key());
            QString trackStatus = da.getCurrentTrackDirectionStatus(direction);
            QByteArray temp_data7;
            temp_data7.resize(3);
            temp_data7[0] = 0x90;
            temp_data7[1] = 0xff;
            //当前轨道正向，且4号道岔期望驱动到定操
            if (trackStatus == "Forward"  && it.value()=="Normal") {
                temp_data7[2] = 0xeb;
                auto crcWrappered = Utilty::appendCRC(temp_data7);
                da.insertCommand(crcWrappered);
                continue;


            }
            //当前轨道正向，且4号道岔期望驱动到反操
            if (trackStatus == "Forward" && it.value() == "Reverse") {
                temp_data7[2] = 0xe7;
                auto crcWrappered = Utilty::appendCRC(temp_data7);
                da.insertCommand(crcWrappered);
                continue;
            }

            if (trackStatus == "Reverse" && it.value() == "Normal") {
                temp_data7[2] = 0xcb;
                auto crcWrappered = Utilty::appendCRC(temp_data7);
                da.insertCommand(crcWrappered);
                continue;
            }

            if (trackStatus == "Reverse" && it.value() == "Reverse") {
                temp_data7[2] = 0xc7;
                auto crcWrappered = Utilty::appendCRC(temp_data7);
                da.insertCommand(crcWrappered);
                continue;
            }
        }
    }

    tempMap.remove("4");

    //对非实物道岔
    if (tempMap.size() > 0) {
        qDebug() << "应该运行到这里了....."<< tempMap.size();
        QStringList siglist = { "2","11","9" };
        QString data1 = "11";
        QString data2="";     
        for (QString the_sig : siglist) {
            if (tempMap.keys().contains(the_sig)==true) {
                auto the_op = tempMap.value(the_sig);
                if (the_op == "Normal") {
                    data1 += "10";
                }
                else
                {
                    data1 += "01";
                }
            }else{
                //需要从这数据库中获取
                auto turnout_status = da.getTrackSectionStatus(the_sig);
                if (turnout_status == "Normal") {
                    data1 += "10";
                }
                else if (turnout_status == "Reverse") {
                    data1 += "01";

                }
                else {
                    data1 += "10";
                }
            }
      
        }

        QStringList siglist1 = { "7","5","3","1" };

        for (QString the_sig : siglist1) {
            if (tempMap.keys().contains(the_sig) == true) {
                auto the_op = tempMap.value(the_sig);
                if (the_op == "Normal") {
                    data2 += "10";
                }
                else
                {
                    data2 += "01";
                }
            }
            else {
                //需要从这数据库中获取
                auto turnout_status = da.getTrackSectionStatus(the_sig);
                if (turnout_status == "Normal") {
                    data2 += "10";
                }
                else if (turnout_status == "Reverse") {
                    data2 += "01";

                }
                else {
                    data2 += "10";
                }
            }

        }
        QByteArray tempba;
        tempba.resize(3);
        tempba[0]= 0x92;
        tempba[1]= Utilty::convertStrToByte(data1.trimmed());;
        tempba[2] = Utilty::convertStrToByte(data2.trimmed());;
        auto crcWrappered = Utilty::appendCRC(tempba);
        qDebug() << ".........." << data1 << data2 << crcWrappered.toHex();
        da.insertCommand(crcWrappered);
    }
    emit turnoutOperationComandSend(routePath,json);
}


void TrackComander::handlerTrackDirectionCheckRequet(QString route_path, QVector<QString> sectionsNeedCheck) {

    QStringList downsectionss = { "3G", "4DG", "IG", "3-11DG" };
    QStringList upsectionss = { "4G", "2DG", "IIG", "1-9DG" };

    for (QString section : sectionsNeedCheck) {

        if (downsectionss.contains(section) == true) {
            QByteArray tempba;
            tempba.resize(3);
            tempba[0] = 0xc8;
            tempba[1] = 0x80;
            tempba[2] = 0x8e;
            da.insertCommandAndGetId(route_path,tempba.toStdString());
        }
        if (upsectionss.contains(section) == true) {
            QByteArray tempba;
            tempba.resize(3);
            tempba[0] = 0xca;
            tempba[1] = 0x80;
            tempba[2] = 0x8e;
            da.insertCommandAndGetId(route_path, tempba.toStdString());

        }


    }


}  //帮助检查所有区段的方向是否已经改变了，为了某个进路的

void TrackComander::handlerSignalColorOperaion(QString routepath, QStringList signallist, QStringList colorList) {
    int sigsize = signallist.size();
    int colsize = colorList.size();
    if (sigsize == colsize) {

        //到底有哪些信号机需要改变颜色的
        QMap<QString, int> whichIDs;
        for (int i = 0; i < sigsize; i++) {
            driverOnePhysignalToSpecificColor(signallist.at(i), colorList.at(i));
            if (signallist.at(i) == "1101" || signallist.at(i) == "XI") {
                QByteArray tempba;
                tempba.resize(3);
                tempba[0] = 0xc1;
                tempba[1] = 0x80;
                tempba[2] = 0x8e;
                auto id=da.insertCommandAndGetId(routepath, tempba.toStdString());
                whichIDs.insert(signallist.at(i), id);
            }

            if (signallist.at(i) == "SI" || signallist.at(i) == "XN" || signallist.at(i) == "X") {
                QByteArray tempba;
                tempba.resize(3);
                tempba[0] = 0xc3;
                tempba[1] = 0x80;
                tempba[2] = 0x8e;
                auto id = da.insertCommandAndGetId(routepath, tempba.toStdString());
                whichIDs.insert(signallist.at(i), id);

            }
            if (signallist.at(i) == "SII" || signallist.at(i) == "S3" || signallist.at(i) == "S4" || signallist.at(i) == "XII") {
                QByteArray tempba;
                tempba.resize(3);
                tempba[0] = 0xc4;
                tempba[1] = 0x80;
                tempba[2] = 0x8e;
                auto id = da.insertCommandAndGetId(routepath, tempba.toStdString());
                whichIDs.insert(signallist.at(i), id);
            }
            if (signallist.at(i) == "X3" || signallist.at(i) == "X4" || signallist.at(i) == "SN" || signallist.at(i) == "S") {
                QByteArray tempba;
                tempba.resize(3);
                tempba[0] = 0xc5;
                tempba[1] = 0x80;
                tempba[2] = 0x8e;
                auto id = da.insertCommandAndGetId(routepath, tempba.toStdString());
                whichIDs.insert(signallist.at(i), id);
            }
            if (signallist.at(i) == "1103" || signallist.at(i) == "1105" || signallist.at(i) == "1107") {
                QByteArray tempba;
                tempba.resize(3);
                tempba[0] = 0xc6;
                tempba[1] = 0x80;
                tempba[2] = 0x8e;
                auto id = da.insertCommandAndGetId(routepath, tempba.toStdString());
                whichIDs.insert(signallist.at(i), id);
            }
            if (signallist.at(i) == "1102" || signallist.at(i) == "1104" || signallist.at(i) == "1106" || signallist.at(i) == "1108") {
                QByteArray tempba;
                tempba.resize(3);
                tempba[0] = 0xc7;
                tempba[1] = 0x80;
                tempba[2] = 0x8e;
                auto id = da.insertCommandAndGetId(routepath, tempba.toStdString());
                whichIDs.insert(signallist.at(i), id);
            }
        }
        //通知Interlockcontroller查询和管理这些，如果所有这些都满足，则进入到发码逻辑里面
        
        emit checkSignalColorStatusForRoute(routepath, signallist, colorList, whichIDs);
        

    }
    


}


void TrackComander::driverOnePhysignalToSpecificColor(QString signal_name, QString wanted_color) {
    if (signal_name == "XI") {
        auto _1101color = this->m_phydevicemanager->getLastedColorForSignal("1101");

        QString data1="";
        if (_1101color->getLight1() == "Red" && _1101color->getLight2() == "Black") {
            data1 = "1011";
        }
        if (_1101color->getLight1() == "Yellow" && _1101color->getLight2() == "Black") {
            data1 = "1101";
        }
        if (_1101color->getLight1() == "Green" && _1101color->getLight2() == "Black") {
            data1 = "1110";
        }

        if (_1101color->getLight1() == "Green" && _1101color->getLight2() == "Yellow") {
            data1 = "1100";
        }
        if (wanted_color == "Green") {
            data1 += "1110";
        }
        else if (wanted_color == "Yellow") {
            data1 += "1101";
        }
        else if (wanted_color == "Red") {
            data1 += "1011";
        }
        else if (wanted_color == "GreenYellow") {
            data1 += "1100";
        }
        else {
            data1 += "1111";
        }
        QString data2="";
        if (_1101color->getLight1() == "Green" && _1101color->getLight2() == "Yellow") {
            data2 = "0111";
        }
        if (_1101color->getLight1() == "Yellow" && _1101color->getLight2() == "Black") {
            data2 = "1011";
        }
        if (_1101color->getLight1() == "Red" && _1101color->getLight2() == "Black") {
            data2 = "1101";
        }
        if (_1101color->getLight1() == "Green" && _1101color->getLight2() == "Black") {
            data2 = "1110";
        }
        if (wanted_color == "Green") {
            data2 += "000";
        }
        else if (wanted_color == "Yellow") {
            data2 += "110";
        }
        else if (wanted_color == "Red") {
            data2 += "111";
        }
        else if (wanted_color == "GreenYellow") {
            data2 += "010";
        }
        else {
            data2 += "111";
        }
        auto the_direction = da.getCurrentTrackDirectionStatus("Down");
        if (the_direction == "Forward") {
            data2 += "1";
        }
        else {
            data2 += "0";
        }

        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x91;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        tempba[1] = byte1;
        tempba[2] = byte2;
        qDebug() << "we obtain the command id is...." << the_id << tempba[1] << tempba[2];

        tempba[3] = 0xff;
        tempba[4] = Utilty::convertStrToByte(data2.trimmed());
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }


    }

    //解决1101的设置

    if (signal_name == "1101") {
        auto _XIcolor = this->m_phydevicemanager->getLastedColorForSignal("XI");
        QString data1 = "";
        if (wanted_color == "Green") {
            data1 += "1110";
        }
        else if (wanted_color == "Yellow") {
            data1 += "1101";
        }
        else if (wanted_color == "Red") {
            data1 += "1011";
        }
        else if (wanted_color == "GreenYellow") {
            data1 += "1100";
        }
        else {
            data1 += "1111";
        }

        if (_XIcolor->getLight1() == "Green" && _XIcolor->getLight2() == "Yellow") {
            data1+= "1100";
        }
        if (_XIcolor->getLight1() == "Yellow" && _XIcolor->getLight2() == "Black") {
            data1 += "1101";
        }
        if (_XIcolor->getLight1() == "Red" && _XIcolor->getLight2() == "Black") {
            data1+= "1011";
        }
        if (_XIcolor->getLight1() == "Green" && _XIcolor->getLight2() == "Black") {
            data1 += "1110";
        }
        QString data2 = "";
        if (wanted_color == "Green") {
            data2 += "1110";
        }
        else if (wanted_color == "Yellow") {
            data2 += "1011";
        }
        else if (wanted_color == "Red") {
            data2 += "1101";
        }
        else if (wanted_color == "GreenYellow") {
            data2 += "0111";
        }
        else {
            data2 += "1111";
        }
        if (_XIcolor->getLight1() == "Green" && _XIcolor->getLight2() == "Black") {
            data2 += "000";
        }
        else if (_XIcolor->getLight1() == "Yellow" && _XIcolor->getLight2() == "Black") {
            data2 += "110";
        }
        else if (_XIcolor->getLight1() == "Green" && _XIcolor->getLight2() == "Yellow") {
            data2 += "010";
        }
        else {
            data2 += "111";
        }
    
        auto the_direction = da.getCurrentTrackDirectionStatus("Down");
        if (the_direction == "Forward") {
            data2 += "1";
        }
        else {
            data2 += "0";
        }
        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x91;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        qDebug() << "we obtain the command id is...." << the_id << byte1<< byte2;
        tempba[1] = byte1;
        tempba[2] = byte2;

        tempba[3] = 0xff;
        tempba[4] = Utilty::convertStrToByte(data2.trimmed());
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }
    }

    if (signal_name == "1107") {
        QString data1 = "11111";
        QString data2 = "11";
        auto _1107_color = wanted_color;
        if (_1107_color == "Yellow") {
            data1 += "011";
        }
        else if (_1107_color == "Green") {
            data1 += "110";
        }
        else if (_1107_color == "Red") {
            data1 += "101";
        }
        else if(_1107_color == "GreenYellow") {
            data1 += "010";
        }
        else {
            data1 += "111";
        }

     
        auto _1105_color = this->m_phydevicemanager->getLastedColorForSignal("1105");
        if (_1105_color->getLight1() == "Yellow" && _1105_color->getLight2() == "Black") {
            data2 += "011";
        }
        if (_1105_color->getLight1() == "Red" && _1105_color->getLight2() == "Black") {
            data2 += "010";
        }
        if (_1105_color->getLight1() == "Green" && _1105_color->getLight2() == "Black") {
            data2 += "110";
        }
        if (_1105_color->getLight1() == "Green" && _1105_color->getLight2() == "Yellow") {
            data2 += "010";
        }

        auto _1103_color = this->m_phydevicemanager->getLastedColorForSignal("1103");
        if (_1103_color->getLight1() == "Yellow" && _1103_color->getLight2() == "Black") {
            data2 += "011";
        }
        if (_1103_color->getLight1() == "Red" && _1103_color->getLight2() == "Black") {
            data2 += "010";
        }
        if (_1103_color->getLight1() == "Green" && _1103_color->getLight2() == "Black") {
            data2 += "110";
        }
        if (_1103_color->getLight1() == "Green" && _1103_color->getLight2() == "Yellow") {
            data2 += "010";
        }

     
        auto firstbyte = Utilty::convertStrToByte(data1);
        auto secondbyte = Utilty::convertStrToByte(data2);
 
        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x96;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);

        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));

        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = firstbyte;
        tempba[4] = secondbyte;
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }


    }

    if (signal_name == "1105") {
        QString data1 = "11111";
        QString data2 = "11";

        auto _1107_color = this->m_phydevicemanager->getLastedColorForSignal("1107");
        if (_1107_color->getLight1() == "Yellow" && _1107_color->getLight2() == "Black") {
            data1 += "011";
        }
        if (_1107_color->getLight1() == "Red" && _1107_color->getLight2() == "Black") {
            data1 += "010";
        }
        if (_1107_color->getLight1() == "Green" && _1107_color->getLight2() == "Black") {
            data1 += "110";
        }
        if (_1107_color->getLight1() == "Green" && _1107_color->getLight2() == "Yellow") {
            data1 += "010";
        }

        auto _1105_color = wanted_color;

        if (_1105_color == "Yellow") {
            data2 += "011";
        }
        else if (_1105_color == "Green") {
            data2 += "110";
        }
        else if (_1105_color == "Red") {
            data2 += "101";
        }
        else if (_1105_color == "GreenYellow") {
            data2 += "010";
        }
        else {
            data2 += "111";
        }
        auto _1103_color = this->m_phydevicemanager->getLastedColorForSignal("1103");
        if (_1103_color->getLight1() == "Yellow" && _1103_color->getLight2() == "Black") {
            data1 += "011";
        }
        if (_1103_color->getLight1() == "Red" && _1103_color->getLight2() == "Black") {
            data1 += "010";
        }
        if (_1103_color->getLight1() == "Green" && _1103_color->getLight2() == "Black") {
            data1 += "110";
        }
        if (_1103_color->getLight1() == "Green" && _1103_color->getLight2() == "Yellow") {
            data1 += "010";
        }

        auto firstbyte = Utilty::convertStrToByte(data1);
        auto secondbyte = Utilty::convertStrToByte(data2);
        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x96;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);

        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));

        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = firstbyte;
        tempba[4] = secondbyte;
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

    }

    if (signal_name == "1103") {
        QString data1 = "11111";
        QString data2 = "11";
        auto _1107_color = this->m_phydevicemanager->getLastedColorForSignal("1107");
        if (_1107_color->getLight1() == "Yellow" && _1107_color->getLight2() == "Black") {
            data1 += "011";
        }
        if (_1107_color->getLight1() == "Red" && _1107_color->getLight2() == "Black") {
            data1 += "010";
        }
        if (_1107_color->getLight1() == "Green" && _1107_color->getLight2() == "Black") {
            data1 += "110";
        }
        if (_1107_color->getLight1() == "Green" && _1107_color->getLight2() == "Yellow") {
            data1 += "010";
        }
        
        auto _1105_color = this->m_phydevicemanager->getLastedColorForSignal("1105");
        if (_1105_color->getLight1() == "Yellow" && _1105_color->getLight2() == "Black") {
            data2 += "011";
        }
        if (_1105_color->getLight1() == "Red" && _1105_color->getLight2() == "Black") {
            data2 += "010";
        }
        if (_1105_color->getLight1() == "Green" && _1105_color->getLight2() == "Black") {
            data2 += "110";
        }
        if (_1105_color->getLight1() == "Green" && _1105_color->getLight2() == "Yellow") {
            data2 += "010";
        }

        auto _1103_color = wanted_color;

        if (_1103_color == "Yellow") {
            data2 += "011";
        }
        else if (_1103_color == "Green") {
            data2 += "110";
        }
        else if (_1103_color == "Red") {
            data2 += "101";
        }
        else if (_1103_color == "GreenYellow") {
            data2 += "010";
        }
        else {
            data2 += "111";
        }

        auto firstbyte = Utilty::convertStrToByte(data1);
        auto secondbyte = Utilty::convertStrToByte(data2);
        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x96;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        tempba[1] = byte1;
        tempba[2] = byte2;

        tempba[3] = firstbyte;
        tempba[4] = secondbyte;
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

    }
    if (signal_name == "1108") {
        QString data1 = "11";
        QString data2 = "11";
        auto _1108_color = wanted_color;

        if (_1108_color == "Yellow") {
            data1 += "011";
        }
        else if (_1108_color == "Green") {
            data1 += "110";
        }
        else if (_1108_color == "Red") {
            data1 += "101";
        }
        else if (_1108_color == "GreenYellow") {
            data1 += "010";
        }
        else {
            data1 += "111";
        }


     
        auto _1106_color = this->m_phydevicemanager->getLastedColorForSignal("1106");

        if (_1106_color->getLight1() == "Yellow" && _1106_color->getLight2() == "Black") {
            data1 += "011";
        }
        if (_1106_color->getLight1() == "Red" && _1106_color->getLight2() == "Black") {
            data1 += "010";
        }
        if (_1106_color->getLight1() == "Green" && _1106_color->getLight2() == "Black") {
            data1 += "110";
        }
        if (_1106_color->getLight1() == "Green" && _1106_color->getLight2() == "Yellow") {
            data1 += "010";
        }


        auto _1104_color = this->m_phydevicemanager->getLastedColorForSignal("1104");

        if (_1104_color->getLight1() == "Yellow" && _1104_color->getLight2() == "Black") {
                data2 += "011";
            }
        if (_1104_color->getLight1() == "Red" && _1104_color->getLight2() == "Black") {
            data2 += "010";
        }
        if (_1104_color->getLight1() == "Green" && _1104_color->getLight2() == "Black") {
            data2 += "110";
        }
        if (_1104_color->getLight1() == "Green" && _1104_color->getLight2() == "Yellow") {
            data2 += "010";
        }




        auto _1102_color = this->m_phydevicemanager->getLastedColorForSignal("1102");

        if (_1102_color->getLight1() == "Yellow" && _1102_color->getLight2() == "Black") {
            data2 += "011";
        }
        if (_1102_color->getLight1() == "Red" && _1102_color->getLight2() == "Black") {
            data2 += "010";
        }
        if (_1102_color->getLight1() == "Green" && _1102_color->getLight2() == "Black") {
            data2 += "110";
        }
        if (_1102_color->getLight1() == "Green" && _1102_color->getLight2() == "Yellow") {
            data2 += "010";
        }
        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x97;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;

        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

    }

    if (signal_name == "1106") {
        QString data1 = "11";
        QString data2 = "11";
        auto _1108_color = this->m_phydevicemanager->getLastedColorForSignal("1108");
        if (_1108_color->getLight1() == "Yellow" && _1108_color->getLight2() == "Black") {
            data1 += "011";
        }
        if (_1108_color->getLight1() == "Red" && _1108_color->getLight2() == "Black") {
            data1 += "010";
        }
        if (_1108_color->getLight1() == "Green" && _1108_color->getLight2() == "Black") {
            data1 += "110";
        }
        if (_1108_color->getLight1() == "Green" && _1108_color->getLight2() == "Yellow") {
            data1 += "010";
        }

    
        auto _1106_color = wanted_color;
        if (_1106_color == "Yellow") {
            data1 += "011";
        }
        else if (_1106_color == "Green") {
            data1 += "110";
        }
        else if (_1106_color == "Red") {
            data1 += "101";
        }
        else if (_1106_color == "GreenYellow") {
            data1 += "010";
        }
        else {
            data1 += "111";
        }


        auto _1104_color = this->m_phydevicemanager->getLastedColorForSignal("1104");
        if (_1104_color->getLight1() == "Yellow" && _1104_color->getLight2() == "Black") {
            data2 += "011";
        }
        if (_1104_color->getLight1() == "Red" && _1104_color->getLight2() == "Black") {
            data2 += "010";
        }
        if (_1104_color->getLight1() == "Green" && _1104_color->getLight2() == "Black") {
            data2 += "110";
        }
        if (_1104_color->getLight1() == "Green" && _1104_color->getLight2() == "Yellow") {
            data2 += "010";
        }

        auto _1102_color = this->m_phydevicemanager->getLastedColorForSignal("1102");
        if (_1102_color->getLight1() == "Yellow" && _1102_color->getLight2() == "Black") {
            data2 += "011";
        }
        if (_1102_color->getLight1() == "Red" && _1102_color->getLight2() == "Black") {
            data2 += "010";
        }
        if (_1102_color->getLight1() == "Green" && _1102_color->getLight2() == "Black") {
            data2 += "110";
        }
        if (_1102_color->getLight1() == "Green" && _1102_color->getLight2() == "Yellow") {
            data2 += "010";
        }
        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x97;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

    }

    if (signal_name == "1104") {
        QString data1 = "11";
        QString data2 = "11";
        auto _1108_color = this->m_phydevicemanager->getLastedColorForSignal("1108");
        if (_1108_color->getLight1() == "Yellow" && _1108_color->getLight2() == "Black") {
            data1 += "011";
        }
        if (_1108_color->getLight1() == "Red" && _1108_color->getLight2() == "Black") {
            data1 += "010";
        }
        if (_1108_color->getLight1() == "Green" && _1108_color->getLight2() == "Black") {
            data1 += "110";
        }
        if (_1108_color->getLight1() == "Green" && _1108_color->getLight2() == "Yellow") {
            data1 += "010";
        }

        auto _1106_color = this->m_phydevicemanager->getLastedColorForSignal("1106");
        if (_1106_color->getLight1() == "Yellow" && _1106_color->getLight2() == "Black") {
            data1 += "011";
        }
        if (_1106_color->getLight1() == "Red" && _1106_color->getLight2() == "Black") {
            data1 += "010";
        }
        if (_1106_color->getLight1() == "Green" && _1106_color->getLight2() == "Black") {
            data1 += "110";
        }
        if (_1106_color->getLight1() == "Green" && _1106_color->getLight2() == "Yellow") {
            data1 += "010";
        }
    
        auto _1104_color = wanted_color;
        if (_1104_color == "Yellow") {
            data2 += "011";
        }
        else if (_1104_color == "Green") {
            data2 += "110";
        }
        else if (_1104_color == "Red") {
            data2 += "101";
        }
        else if (_1104_color == "GreenYellow") {
            data2 += "010";
        }
        else {
            data2 += "111";
        }


        auto _1102_color = this->m_phydevicemanager->getLastedColorForSignal("1102");
        if (_1102_color->getLight1() == "Yellow" && _1102_color->getLight2() == "Black") {
            data2 += "011";
        }
        if (_1102_color->getLight1() == "Red" && _1102_color->getLight2() == "Black") {
            data2 += "010";
        }
        if (_1102_color->getLight1() == "Green" && _1102_color->getLight2() == "Black") {
            data1 += "110";
        }
        if (_1102_color->getLight1() == "Green" && _1102_color->getLight2() == "Yellow") {
            data1 += "010";
        }

        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x97;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

    }

    if (signal_name == "1102") {
        QString data1 = "11";
        QString data2 = "11";

        auto _1108_color = this->m_phydevicemanager->getLastedColorForSignal("1108");
        if (_1108_color->getLight1() == "Yellow" && _1108_color->getLight2() == "Black") {
            data1 += "011";
        }
        if (_1108_color->getLight1() == "Red" && _1108_color->getLight2() == "Black") {
            data1 += "010";
        }
        if (_1108_color->getLight1() == "Green" && _1108_color->getLight2() == "Black") {
            data1 += "110";
        }
        if (_1108_color->getLight1() == "Green" && _1108_color->getLight2() == "Yellow") {
            data1 += "010";
        }

        auto _1106_color = this->m_phydevicemanager->getLastedColorForSignal("1106");
        if (_1106_color->getLight1() == "Yellow" && _1106_color->getLight2() == "Black") {
            data1 += "011";
        }
        if (_1106_color->getLight1() == "Red" && _1106_color->getLight2() == "Black") {
            data1 += "010";
        }
        if (_1106_color->getLight1() == "Green" && _1106_color->getLight2() == "Black") {
            data1 += "110";
        }
        if (_1106_color->getLight1() == "Green" && _1106_color->getLight2() == "Yellow") {
            data1 += "010";
        }
  
        auto _1104_color = this->m_phydevicemanager->getLastedColorForSignal("1104");
        if (_1104_color->getLight1() == "Yellow" && _1104_color->getLight2() == "Black") {
            data2 += "011";
        }
        if (_1104_color->getLight1() == "Red" && _1104_color->getLight2() == "Black") {
            data2 += "010";
        }
        if (_1104_color->getLight1() == "Green" && _1104_color->getLight2() == "Black") {
            data2 += "110";
        }
        if (_1104_color->getLight1() == "Green" && _1104_color->getLight2() == "Yellow") {
            data2 += "010";
        }

        auto _1102_color = wanted_color;
        if (_1102_color == "Yellow") {
            data2 += "011";
        }
        else if (_1102_color == "Green") {
            data2 += "110";
        }
        else if (_1102_color == "Red") {
            data2 += "101";
        }
        else if (_1102_color == "GreenYellow") {
            data2 += "010";
        }
        else {
            data2 += "111";
        }

        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x97;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        tempba[1] = byte1;
        tempba[2] = byte2;

        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

    }

    if (signal_name == "X4") {
        QString data1 = "1";
        QString data2 = "";
        auto _X4_color = wanted_color;
        if (wanted_color == "Red") {
            data1 += "01";
        }
        else if (wanted_color == "Green") {
            data1 += "10";
        }
        else {
            data1 += "11";
        }

        auto _X3_color = this->m_phydevicemanager->getLastedColorForSignal("X3");
        if (_X3_color->getLight1() == "Yellow" && _X3_color->getLight2() == "Black") {
            data1 += "011";
        }
        if (_X3_color->getLight1() == "Red" && _X3_color->getLight2() == "Black") {
            data1 += "101";
        }
        if (_X3_color->getLight1() == "Green" && _X3_color->getLight2() == "Black") {
            data1 += "001";
        }

        auto _SN_color = this->m_phydevicemanager->getLastedColorForSignal("SN");
        if (_SN_color->getLight1() == "White" && _SN_color->getLight2() == "Black") {
            data1 += "01";
        }else if (_SN_color->getLight1() == "Yellow" && _SN_color->getLight2() == "Yellow") {
            data1 += "10";
        }
        else {
            data1 += "11";
        }
        if (_SN_color->getLight1() == "Red" && _SN_color->getLight2() == "Black") {
            data2 += "011";
        } else if (_SN_color->getLight1() == "Green" && _SN_color->getLight2() == "Black") {
            data2 += "101";
        }
        else if (_SN_color->getLight1() == "Yellow" && _SN_color->getLight2() == "Black") {
            data2 += "110";
        }
        else {
            data2 += "111";
        }
   

        auto _S_color = this->m_phydevicemanager->getLastedColorForSignal("S");
        if (_S_color->getLight1() == "White" && _S_color->getLight2() == "Black") {
            data2 += "01111";
        } else  if (_S_color->getLight1() == "Yellow" && _S_color->getLight2() == "Yellow") {
            data2 += "10111";
        }else  if (_S_color->getLight1() == "Red" && _S_color->getLight2() == "Black") {
            data2 += "11011";
        }
        else  if (_S_color->getLight1() == "Green" && _S_color->getLight2() == "Black") {
            data2 += "11101";
        }
        else  if (_S_color->getLight1() == "Yellow" && _S_color->getLight2() == "Black") {
            data2 += "11110";
        }
        else  if (_S_color->getLight1() == "Red" && _S_color->getLight2() == "White") {
            data2 += "01011";
        }
        else {
            data2 += "11111";
        }
        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x95;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

    }

    if (signal_name == "X3") {
        QString data1 = "1";
        QString data2 = "";
        auto _X4_color = this->m_phydevicemanager->getLastedColorForSignal("X4");
        if (_X4_color->getLight1() == "Red" && _X4_color->getLight2() == "Black") {
            data1 += "01";
        }
        else if (_X4_color->getLight1() == "Green" && _X4_color->getLight2() == "Black") {    
            data1 += "10";
        }
        else {
            data1 += "11";   
        }
        auto _X3_color = wanted_color;
        if (wanted_color == "Yellow") {
            data1 += "011";
        }
        else if (wanted_color == "Red") {
            data1 += "101";
        }
        else if (wanted_color == "Green") {
            data1 += "110";
        }
        else {
            data1 += "111";
        }

        auto _SN_color = this->m_phydevicemanager->getLastedColorForSignal("SN");
        if (_SN_color->getLight1() == "White" && _SN_color->getLight2() == "Black") {
            data1 += "01";
            data2 += "111";
        }
        else if (_SN_color->getLight1() == "Yellow" && _SN_color->getLight2() == "Yellow") {
            data1 += "10";
            data2 += "111";
        }
        else if (_SN_color->getLight1() == "Red" && _SN_color->getLight2() == "Black") {
            data1 += "11";
            data2 += "011";
        }
        else if (_SN_color->getLight1() == "Green" && _SN_color->getLight2() == "Black") {
            data1 += "11";
            data2 += "101";
        }
        else if (_SN_color->getLight1() == "Yellow" && _SN_color->getLight2() == "Black") {
            data1 += "11";
            data2 += "110";
        }
        else {
            data1 += "11";
            data2 += "111";
        }

        auto _S_color = this->m_phydevicemanager->getLastedColorForSignal("S");
        if (_S_color->getLight1() == "White" && _S_color->getLight2() == "Black") {
            data2 += "01111";
        }
        else if (_S_color->getLight1() == "Yellow" && _S_color->getLight2() == "Yellow") {
            data2 += "10111";
        }
        else if (_S_color->getLight1() == "Red" && _S_color->getLight2() == "Black") {
            data2 += "11011";
        }
        else if (_S_color->getLight1() == "Green" && _S_color->getLight2() == "Black") {
            data2 += "11101";
        }
        else if (_S_color->getLight1() == "Yellow" && _S_color->getLight2() == "Black") {
            data2 += "11110";
        }
        else {
            data2 += "11111";
        }

        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x95;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));


        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }
        ;
        return;
    }

    if (signal_name == "SN") {
        QString data1 = "1";
        QString data2 = "";
        auto _X4_color = this->m_phydevicemanager->getLastedColorForSignal("X4");
        if (_X4_color->getLight1() == "Red" && _X4_color->getLight2() == "Black") {
            data1 += "01";
        }else if (_X4_color->getLight1() == "Green" && _X4_color->getLight2() == "Black") {
            data1 += "10";
        }
        else {
            data1 += "11";
        }

        auto _X3_color = this->m_phydevicemanager->getLastedColorForSignal("X3");
        if (_X3_color->getLight1() == "Yellow" && _X3_color->getLight2() == "Black") {
            data1 += "011";
        }else if (_X3_color->getLight1() == "Red" && _X3_color->getLight2() == "Black") {
            data1 += "101";
        }
        else  if (_X3_color->getLight1() == "Green" && _X3_color->getLight2() == "Black") {
            data1 += "110";
        }
        else {
            data1 += "111";
        }

      
        auto _SN_color = wanted_color;
        if (_SN_color == "White") {
            data1 += "01";
            data2 += "111";
        }else     if (_SN_color == "YellowYellow") {
            data1 += "10";
            data2 += "111";
        }
        else     if (_SN_color == "Red") {
            data1 += "11";
            data2 += "011";
        }
        else     if (_SN_color == "Green") {
            data1 += "11";
            data2 += "101";
        }
        else     if (_SN_color == "Yellow") {
            data1 += "11";
            data2 += "110";
        }
        else {
            data1 += "11";
            data2 += "111";
        }


        auto _S_color = this->m_phydevicemanager->getLastedColorForSignal("S");
        if (_S_color->getLight1() == "White" && _S_color->getLight2() == "Black") {
            data2 += "01111";
        }
        else if (_S_color->getLight1() == "Yellow" && _S_color->getLight2() == "Yellow") {
            data2 += "10111";
        }
        else if (_S_color->getLight1() == "Red" && _S_color->getLight2() == "Black") {
            data2 += "11011";
        }
        else if (_S_color->getLight1() == "Green" && _S_color->getLight2() == "Black") {
            data2 += "11101";
        }
        else if (_S_color->getLight1() == "Yellow" && _S_color->getLight2() == "Black") {
            data2 += "11110";
        }
        else {
            data2 += "11111";
        }

        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x95;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

        return;
    }
    if (signal_name == "S") {
        QString data1 = "1";
        QString data2 = "";
        auto _X4_color = this->m_phydevicemanager->getLastedColorForSignal("X4");
        if (_X4_color->getLight1() == "Red" && _X4_color->getLight2() == "Black") {
            data1 += "01";
        }
        else if (_X4_color->getLight1() == "Green" && _X4_color->getLight2() == "Black") {
            data1 += "10";
        }
        else {
            data1 += "11";
        }
        auto _X3_color = this->m_phydevicemanager->getLastedColorForSignal("X3");
        if (_X3_color->getLight1() == "Yellow" && _X3_color->getLight2() == "Black") {
            data1 += "011";
        }
        else if (_X3_color->getLight1() == "Red" && _X3_color->getLight2() == "Black") {
            data1 += "101";
        }
        else  if (_X3_color->getLight1() == "Green" && _X3_color->getLight2() == "Black") {
            data1 += "110";
        }
        else {
            data1 += "111";
        }

        auto _SN_color = this->m_phydevicemanager->getLastedColorForSignal("SN");
        if (_SN_color->getLight1() == "White" && _SN_color->getLight2() == "Black") {
            data1 += "01";
            data2 += "111";
        }
        else if (_SN_color->getLight1() == "Yellow" && _SN_color->getLight2() == "Yellow") {
            data1 += "10";
            data2 += "111";
        }
        else if (_SN_color->getLight1() == "Red" && _SN_color->getLight2() == "Black") {
            data1 += "11";
            data2 += "011";
        }
        else if (_SN_color->getLight1() == "Green" && _SN_color->getLight2() == "Black") {
            data1 += "11";
            data2 += "101";
        }
        else if (_SN_color->getLight1() == "Yellow" && _SN_color->getLight2() == "Black") {
            data1 += "11";
            data2 += "110";
        }
        else {
            data1 += "11";
            data2 += "111";
        }

  
        auto _S_color = wanted_color;
        if (_S_color == "White") {
            data2 += "01111";
        }else  if (_S_color == "YellowYellow") {
            data2 += "10111";
        }
        else  if (_S_color == "Red") {
            data2 += "11011";
        }
        else  if (_S_color == "Green") {
            data2 += "11101";
        }
        else  if (_S_color == "Yellow") {
            data2 += "11110";
        }
        else {
            data2 += "11111";
        }
        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x95;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

        return;
    }
    if (signal_name == "XII") {
        QString data1 = "11";
        QString data2 = "";
        auto _XII_color = wanted_color;
        if (_XII_color == "Red") {
            data1 += "01";
        }else if (_XII_color == "Green") {
            data1 += "10";
        }
        else {
            data1 += "11";
        }

        auto _S4_color = this->m_phydevicemanager->getLastedColorForSignal("S4");
        if (_S4_color->getLight1() == "White" && _S4_color->getLight1() == "Red") {
            data1 += "0101";
        }else      if (_S4_color->getLight1() == "White" && _S4_color->getLight2() == "Black") {
            data1 += "0111";
        }
        else      if (_S4_color->getLight1() == "Yellow" && _S4_color->getLight2() == "Black") {
            data1 += "1011";
        }
        else      if (_S4_color->getLight1() == "Red" && _S4_color->getLight2() == "Black") {
            data1 += "1101";
        }
        else      if (_S4_color->getLight1() == "Green" && _S4_color->getLight2() == "Black") {
            data1 += "1110";
        }
        else {
            data1 += "1111";

        }

        auto _S3_color = this->m_phydevicemanager->getLastedColorForSignal("S3");
        if (_S3_color->getLight1() == "White" && _S3_color->getLight2() == "Red") {
            data2 += "0101";
        }
        else      if (_S3_color->getLight1() == "White" && _S3_color->getLight2() == "Black") {
            data2 += "0111";
        }
        else      if (_S3_color->getLight1() == "Yellow" && _S3_color->getLight2() == "Black") {
            data2 += "1011";
        }
        else      if (_S3_color->getLight1() == "Red" && _S3_color->getLight2() == "Black") {
            data2 += "1101";
        }
        else      if (_S3_color->getLight1() == "Green" && _S3_color->getLight2() == "Black") {
            data2 += "1110";
        }
        else {
            data2 += "1111";

        }

        auto _SII_color = this->m_phydevicemanager->getLastedColorForSignal("SII");
        if (_SII_color->getLight1() == "White" && _SII_color->getLight2() == "Red") {
            data2 += "0101";
        }
        else      if (_SII_color->getLight1() == "White" && _SII_color->getLight2() == "Black") {
            data2 += "0111";
        }
        else      if (_SII_color->getLight1() == "Yellow" && _SII_color->getLight2() == "Black") {
            data2 += "1011";
        }
        else      if (_SII_color->getLight1() == "Red" && _SII_color->getLight2() == "Black") {
            data2 += "1101";
        }
        else      if (_SII_color->getLight1() == "Green" && _SII_color->getLight2() == "Black") {
            data2 += "1110";
        }
        else {
            data2 += "1111";

        }

       
        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x95;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));


        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

        return;
    }
    if (signal_name == "S4") {
        QString data1 = "11";
        QString data2 = "";


        auto _XII_color = this->m_phydevicemanager->getLastedColorForSignal("XII");
        if (_XII_color->getLight1() == "Red" && _XII_color->getLight1() == "Black") {
            
            data1 += "10";
        }else  if (_XII_color->getLight1() == "Green" && _XII_color->getLight1() == "Black") {

            data1 += "01";
        }
        else {
            data1 += "11";

        
        }
        auto _S4_color = wanted_color;
        if (_S4_color == "White") {
            data1 += "01111";
        }
        else  if (_S4_color == "YellowYellow") {
            data1 += "10111";
        }
        else  if (_S4_color == "Red") {
            data1 += "11011";
        }
        else  if (_S4_color == "Green") {
            data1 += "11101";
        }
        else  if (_S4_color == "Yellow") {
            data1 += "11110";
        }
        else {
            data1 += "11111";
        }

        auto _S3_color = this->m_phydevicemanager->getLastedColorForSignal("S3");
        if (_S3_color->getLight1() == "White" && _S3_color->getLight2() == "Red") {
            data2 += "0101";
        }
        else      if (_S3_color->getLight1() == "White" && _S3_color->getLight2() == "Black") {
            data2 += "0111";
        }
        else      if (_S3_color->getLight1() == "Yellow" && _S3_color->getLight2() == "Black") {
            data2 += "1011";
        }
        else      if (_S3_color->getLight1() == "Red" && _S3_color->getLight2() == "Black") {
            data2 += "1101";
        }
        else      if (_S3_color->getLight1() == "Green" && _S3_color->getLight2() == "Black") {
            data2 += "1110";
        }
        else {
            data2 += "1111";

        }

        auto _SII_color = this->m_phydevicemanager->getLastedColorForSignal("SII");
        if (_SII_color->getLight1() == "White" && _SII_color->getLight2() == "Red") {
            data2 += "0101";
        }
        else      if (_SII_color->getLight1() == "White" && _SII_color->getLight2() == "Black") {
            data2 += "0111";
        }
        else      if (_SII_color->getLight1() == "Yellow" && _SII_color->getLight2() == "Black") {
            data2 += "1011";
        }
        else      if (_SII_color->getLight1() == "Red" && _SII_color->getLight2() == "Black") {
            data2 += "1101";
        }
        else      if (_SII_color->getLight1() == "Green" && _SII_color->getLight2() == "Black") {
            data2 += "1110";
        }
        else {
            data2 += "1111";

        }
        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x94;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

        return;
    }

    if (signal_name == "S3") {
        QString data1 = "11";
        QString data2 = "";
        auto _XII_color = this->m_phydevicemanager->getLastedColorForSignal("XII");
        if (_XII_color->getLight1() == "Red" && _XII_color->getLight1() == "Black") {

            data1 += "10";
        }
        else  if (_XII_color->getLight1() == "Green" && _XII_color->getLight1() == "Black") {

            data1 += "01";
        }
        else {
            data1 += "11";


        };

        auto _S4_color = this->m_phydevicemanager->getLastedColorForSignal("S4");
        if (_S4_color->getLight1() == "White" && _S4_color->getLight1() == "Red") {
            data1 += "0101";
        }
        else      if (_S4_color->getLight1() == "White" && _S4_color->getLight2() == "Black") {
            data1 += "0111";
        }
        else      if (_S4_color->getLight1() == "Yellow" && _S4_color->getLight2() == "Black") {
            data1 += "1011";
        }
        else      if (_S4_color->getLight1() == "Red" && _S4_color->getLight2() == "Black") {
            data1 += "1101";
        }
        else      if (_S4_color->getLight1() == "Green" && _S4_color->getLight2() == "Black") {
            data1 += "1110";
        }
        else {
            data1 += "1111";

        }

        
        auto _S3_color = wanted_color;
        if (_S3_color == "White") {
            data2 += "0111";
        }
        else      if (_S3_color == "Yellow") {
            data2 += "1011";
        }
        else      if (_S3_color == "Red") {
            data2 += "1101";
        }
        else      if (_S3_color == "Green") {
            data2 += "1110";
        } if (_S3_color == "WhiteRed") {
            data2 += "0101";
        }
        else {
            data2 += "1111";

        }

        auto _SII_color = this->m_phydevicemanager->getLastedColorForSignal("SII");
        if (_SII_color->getLight1() == "White" && _SII_color->getLight2() == "Red") {
            data2 += "0101";
        }
        else      if (_SII_color->getLight1() == "White" && _SII_color->getLight2() == "Black") {
            data2 += "0111";
        }
        else      if (_SII_color->getLight1() == "Yellow" && _SII_color->getLight2() == "Black") {
            data2 += "1011";
        }
        else      if (_SII_color->getLight1() == "Red" && _SII_color->getLight2() == "Black") {
            data2 += "1101";
        }
        else      if (_SII_color->getLight1() == "Green" && _SII_color->getLight2() == "Black") {
            data2 += "1110";
        }
        else {
            data2 += "1111";

        }


        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x94;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

        return;
    }

    if (signal_name == "SII") {
        QString data1 = "11";
        QString data2 = "";
        auto _XII_color = this->m_phydevicemanager->getLastedColorForSignal("XII");
        if (_XII_color->getLight1() == "Red" && _XII_color->getLight1() == "Black") {

            data1 += "10";
        }
        else  if (_XII_color->getLight1() == "Green" && _XII_color->getLight1() == "Black") {

            data1 += "01";
        }
        else {
            data1 += "11";


        };

        auto _S4_color = this->m_phydevicemanager->getLastedColorForSignal("S4");
        if (_S4_color->getLight1() == "White" && _S4_color->getLight1() == "Red") {
            data1 += "0101";
        }
        else      if (_S4_color->getLight1() == "White" && _S4_color->getLight2() == "Black") {
            data1 += "0111";
        }
        else      if (_S4_color->getLight1() == "Yellow" && _S4_color->getLight2() == "Black") {
            data1 += "1011";
        }
        else      if (_S4_color->getLight1() == "Red" && _S4_color->getLight2() == "Black") {
            data1 += "1101";
        }
        else      if (_S4_color->getLight1() == "Green" && _S4_color->getLight2() == "Black") {
            data1 += "1110";
        }
        else {
            data1 += "1111";

        }


        auto _S3_color = this->m_phydevicemanager->getLastedColorForSignal("S3");
        if (_S3_color->getLight1() == "White" && _S3_color->getLight1() == "Red") {
            data2 += "0101";
        }
        else      if (_S3_color->getLight1() == "White" && _S3_color->getLight2() == "Black") {
            data2 += "0111";
        }
        else      if (_S3_color->getLight1() == "Yellow" && _S3_color->getLight2() == "Black") {
            data2 += "1011";
        }
        else      if (_S3_color->getLight1() == "Red" && _S3_color->getLight2() == "Black") {
            data2 += "1101";
        }
        else      if (_S3_color->getLight1() == "Green" && _S3_color->getLight2() == "Black") {
            data2 += "1110";
        }
        else {
            data2 += "1111";

        }
        
        auto _SII_color = wanted_color;
        if (_SII_color == "Red") {
            data2 += "1101";
        }
        else if (_SII_color=="Yellow") {
            data2 += "1011";
        }
        else if (_SII_color == "Green") {
            data2 += "1110";
        }
        else if (_SII_color == "WhiteRed") {
            data2 += "0101";
        }
        else if (_SII_color == "White") {
            data2 += "0111";
        }
        else {
            data2 += "1111";
        }


        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x94;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));

        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

        return;
    }

    if (signal_name == "SI") {
        QString data1 = "11";
        QString  data2 = "";
        auto _SI_color = wanted_color;
        if (_SI_color == "White") {
            data1 += "0111";
        }else if (_SI_color == "Yellow") {
            data1 += "1011";
        }
        else if (_SI_color == "Red") {
            data1 += "1101";
        }
        else if (_SI_color == "Green") {
            data1 += "1110";
        }
        else if (_SI_color == "WhiteRed") {
            data1 += "0101";
        }
        else {
            data1 += "1111";
        }
        auto _XN_color = this->m_phydevicemanager->getLastedColorForSignal("XN");
        if (_XN_color->getLight1() == "White" && _XN_color->getLight2() == "Black") {
            data1 += "01";
            data2 += "111";
        }
        else  if (_XN_color->getLight1() == "White" && _XN_color->getLight2() == "Red") {
            data1 += "01";
            data2 += "011";
        }
        else  if (_XN_color->getLight1() == "Yellow" && _XN_color->getLight2() == "Yellow") {
            data1 += "10";
            data2 += "111";
        }
        else  if (_XN_color->getLight1() == "Red" && _XN_color->getLight2() == "Black") {
            data1 += "11";
            data2 += "011";
        }
        else  if (_XN_color->getLight1() == "Green" && _XN_color->getLight2() == "Black") {
            data1 += "11";
            data2 += "101";
        }
        else  if (_XN_color->getLight1() == "Yellow" && _XN_color->getLight2() == "Black") {
            data1 += "11";
            data2 += "110";
        }
        else {
            data1 += "11";
            data2 += "111";
        }

        auto _X_color = this->m_phydevicemanager->getLastedColorForSignal("X");
        if (_X_color->getLight1() == "White" && _X_color->getLight2() == "Black") {
            data2 += "01";
            data2 += "111";
        }
        else  if (_X_color->getLight1() == "White" && _X_color->getLight2() == "Red") {
            data2 += "01";
            data2 += "011";
        }
        else  if (_X_color->getLight1() == "Yellow" && _X_color->getLight2() == "Yellow") {
            data2 += "10";
            data2 += "111";
        }
        else  if (_X_color->getLight1() == "Red" && _X_color->getLight2() == "Black") {
            data2 += "11";
            data2 += "011";
        }
        else  if (_X_color->getLight1() == "Green" && _X_color->getLight2() == "Black") {
            data2 += "11";
            data2 += "101";
        }
        else  if (_X_color->getLight1() == "Yellow" && _X_color->getLight2() == "Black") {
            data2 += "11";
            data2 += "110";
        }
        else {
            data2 += "11";
            data2 += "111";
        }
       
        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x93;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));

        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

        return;
    }
    if (signal_name == "XN") {
        QString data1 = "11";
        QString  data2 = "";


        auto _SI_color = this->m_phydevicemanager->getLastedColorForSignal("SI");
        if (_SI_color->getLight1() == "White" && _SI_color->getLight2() == "Black") {
            data1 += "01";
            data1 += "111";
        }
        else  if (_SI_color->getLight1() == "White" && _SI_color->getLight2() == "Red") {
            data1 += "01";
            data1 += "011";
        }
        else  if (_SI_color->getLight1() == "Yellow" && _SI_color->getLight2() == "Yellow") {
            data1 += "10";
            data1 += "111";
        }
        else  if (_SI_color->getLight1() == "Red" && _SI_color->getLight2() == "Black") {
            data1 += "11";
            data1 += "011";
        }
        else  if (_SI_color->getLight1() == "Green" && _SI_color->getLight2() == "Black") {
            data1 += "11";
            data1 += "101";
        }
        else  if (_SI_color->getLight1() == "Yellow" && _SI_color->getLight2() == "Black") {
            data1 += "11";
            data1 += "110";
        }
        else {
            data1 += "11";
            data1 += "111";
        }


        auto _XN_color = wanted_color;

        if (_XN_color == "White") {
            data1 += "01";
            data2 += "111";
        }
        else if (_XN_color == "Yellow") {
            data1 += "11";
            data2 += "110";
        }
        else if (_XN_color == "Red") {
            data1 += "11";
            data2 += "011";
        }
        else if (_XN_color == "Green") {
            data1 += "11";
            data2 += "101";
        }
        else if (_XN_color == "WhiteRed") {
            data1 += "01";
            data2 += "011";
        }
        else {
            data1 += "11";
            data2 += "111";
        }

  
        auto _X_color = this->m_phydevicemanager->getLastedColorForSignal("X");
        if (_X_color->getLight1() == "White" && _X_color->getLight2() == "Black") {
            data2 += "01";
            data2 += "111";
        }
        else  if (_X_color->getLight1() == "White" && _X_color->getLight2() == "Red") {
            data2 += "01";
            data2 += "011";
        }
        else  if (_X_color->getLight1() == "Yellow" && _X_color->getLight2() == "Yellow") {
            data2 += "10";
            data2 += "111";
        }
        else  if (_X_color->getLight1() == "Red" && _X_color->getLight2() == "Black") {
            data2 += "11";
            data2 += "011";
        }
        else  if (_X_color->getLight1() == "Green" && _X_color->getLight2() == "Black") {
            data2 += "11";
            data2 += "101";
        }
        else  if (_X_color->getLight1() == "Yellow" && _X_color->getLight2() == "Black") {
            data2 += "11";
            data2 += "110";
        }
        else {
            data2 += "11";
            data2 += "111";
        }
        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x93;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        qDebug() << "we obtain the command id is...." << the_id;
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));

        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }

        return;
    }
    if (signal_name == "X") {
        QString data1 = "11";
        QString  data2 = "";

        auto _SI_color = this->m_phydevicemanager->getLastedColorForSignal("SI");
        if (_SI_color->getLight1() == "White" && _SI_color->getLight2() == "Black") {
            data1 += "01";
            data1 += "111";
        }
        else  if (_SI_color->getLight1() == "White" && _SI_color->getLight2() == "Red") {
            data1 += "01";
            data1 += "011";
        }
        else  if (_SI_color->getLight1() == "Yellow" && _SI_color->getLight2() == "Yellow") {
            data1 += "10";
            data1 += "111";
        }
        else  if (_SI_color->getLight1() == "Red" && _SI_color->getLight2() == "Black") {
            data1 += "11";
            data1 += "011";
        }
        else  if (_SI_color->getLight1() == "Green" && _SI_color->getLight2() == "Black") {
            data1 += "11";
            data1 += "101";
        }
        else  if (_SI_color->getLight1() == "Yellow" && _SI_color->getLight2() == "Black") {
            data1 += "11";
            data1 += "110";
        }
        else {
            data1 += "11";
            data1 += "111";
        }


        auto _XN_color = this->m_phydevicemanager->getLastedColorForSignal("XN");
        if (_XN_color->getLight1() == "White" && _XN_color->getLight2() == "Black") {
            data1 += "01";
            data2 += "111";
        }
        else  if (_XN_color->getLight1() == "White" && _XN_color->getLight2() == "Red") {
            data1 += "01";
            data2 += "011";
        }
        else  if (_XN_color->getLight1() == "Yellow" && _XN_color->getLight2() == "Yellow") {
            data1 += "10";
            data2 += "111";
        }
        else  if (_XN_color->getLight1() == "Red" && _XN_color->getLight2() == "Black") {
            data1 += "11";
            data2 += "011";
        }
        else  if (_XN_color->getLight1() == "Green" && _XN_color->getLight2() == "Black") {
            data1 += "11";
            data2 += "101";
        }
        else  if (_XN_color->getLight1() == "Yellow" && _XN_color->getLight2() == "Black") {
            data1 += "11";
            data2 += "110";
        }
        else {
            data1 += "11";
            data2 += "111";
        }


        auto _X_color = wanted_color;


        if (_X_color == "White") {
            data2 += "01";
            data2 += "111";
        }
        else if (_X_color == "Yellow") {
            data2 += "11";
            data2 += "110";
        }
        else if (_X_color == "Red") {
            data2 += "11";
            data2 += "011";
        }
        else if (_X_color == "Green") {
            data2 += "11";
            data2 += "101";
        }
        else if (_X_color == "WhiteRed") {
            data2 += "01";
            data2 += "011";
        }
        else {
            data2 += "11";
            data2 += "111";
        }

        QByteArray tempba;
        tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
        tempba[0] = 0x93;
        da.db.transaction();
        auto the_id = da.insertCommandWithoutContent();
        QString binaryStr = Utilty::intToQString(the_id);
        auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
        auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
        qDebug() << "we obtain the command id is...." << the_id;
        tempba[1] = byte1;
        tempba[2] = byte2;
        tempba[3] = Utilty::convertStrToByte(data1);
        tempba[4] = Utilty::convertStrToByte(data2);
        auto wcrc = Utilty::ModbusCRC16(tempba.left(5));
        auto crcdi = uint8_t(wcrc);
        auto crcgao = uint8_t(wcrc >> 8);
        tempba[5] = crcdi;
        tempba[6] = crcgao;
        // 更新数据库记录
        if (da.updateCommandRecord(the_id, tempba)) {
            // 提交事务
            da.db.commit();
        }
        else {
            // 回滚事务
            da.db.rollback();
        }


        return;
    }
}



void TrackComander::FaMaHandler(QString segments, QString famas, QString uplinkornot) {
    QString the_sgeco = "";
    QString maxu = "";
    if (uplinkornot == "Down") {
        the_sgeco ="X1LQG|1101G|1103G|1105G|1107G|3-11G|1G|4DG|3G";
    }
    else {
        the_sgeco = "S1LQG|1108G|1106G|1104G|1102G|2DG|IIG|1-9DG|4G";
    }
    auto the_segs_vec = segments.split("|");
    auto the_fama_vec = famas.split("|");
    for (QString seghhh : the_sgeco.split("|")) {
        int indexs = 0;
        for (QString innn : the_segs_vec) {
            if (innn != seghhh) {
                indexs = indexs + 1;
            }
            else {
                break;
            }
        }
        auto famaaaa = the_fama_vec.at(indexs);
        maxu += famaaaa;
        maxu += "|";
    }
    maxu.truncate(maxu.lastIndexOf(QChar('|')));
    qDebug()<< "adjust maxu.... for segment"<< the_sgeco <<"..."<<maxu;
    communicationFaMa(Utilty::removeLastSpiltCharacter(maxu), uplinkornot);
}




bool TrackComander::communicationFaMa(QString fama, QString direction) {
    QByteArray data;
    data.resize(10);

    // 第一个字节
    if (direction == "Down") {
        data[0] = 0xfa;
    }
    else {
        data[0] = 0xfb;
    }
    
    //先生成唯一ID
    da.db.transaction();
    auto the_id = da.insertCommandWithoutContent();
    qDebug() << "we obtain the command id is...." << the_id;
    QString binaryStr = Utilty::intToQString(the_id);
    auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
    auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));

    data[1] = byte1;
    data[2] = byte2;
    auto zz = fama.split("|");
    char tempbytearr[9];   
    for (int i = 0; i < zz.length(); i++) {
        if (zz[i] == "L") {
            tempbytearr[i] = 0x00;
        }
        else if (zz[i] == "LU") {
            tempbytearr[i] = 0x01;
        }
        else if (zz[i] == "U") {
            tempbytearr[i] = 0x02;
        }
        else if (zz[i] == "UU") {
            tempbytearr[i] = 0x03;
        }
        else if (zz[i] == "U2S") {
            tempbytearr[i] = 0x04;
        }
        else if (zz[i] == "U2") {
            tempbytearr[i] = 0x05;
        }
        else if (zz[i] == "UUS") {
            tempbytearr[i] = 0x06;
        }
        else if (zz[i] == "HB") {
            tempbytearr[i] = 0x07;
        }
        else if (zz[i] == "HU") {
            tempbytearr[i] = 0x08;
        }
        else if (zz[i] == "LU2") {
            tempbytearr[i] = 0x09;
        }
        else if (zz[i] == "ZQ") { //载频切换
            tempbytearr[i] = 0x0a;
        }
        else if (zz[i] == "JC") { //占用检查
            tempbytearr[i] = 0x0b;
        }
        else if (zz[i] == "H") {
            tempbytearr[i] = 0x0c;
        }
        else if (zz[i] == "L2") {
            tempbytearr[i] = 0x0d;
        }
        else if (zz[i] == "L3") {
            tempbytearr[i] = 0x0e;
        }
        else if (zz[i] == "L4") {
            tempbytearr[i] = 0x0f;
        }
        else if (zz[i] == "L5") {
            tempbytearr[i] = 0x10;
        }
        else if (zz[i] == "L6") {
            tempbytearr[i] = 0x11;
        }
        else if (zz[i] == "ZY") {
            tempbytearr[i] = 0x0b;
        }
        else {
            tempbytearr[i] = 0x0b;
        }
    }
    data[3] = (char)(((tempbytearr[0] << 4) & 0xff) | tempbytearr[1]);
    data[4] = (char)(((tempbytearr[2] << 4) & 0xff) | tempbytearr[3]);
    data[5] = (char)(((tempbytearr[4] << 4) & 0xff) | tempbytearr[5]);
    data[6] = (char)(((tempbytearr[6] << 4) & 0xff) | tempbytearr[7]);
    data[7] = (char)(((tempbytearr[8] << 4) & 0xff));
    
    uint16_t wcrc = Utilty::ModbusCRC16(data.left(8));
    unsigned char crcdi = uint8_t(wcrc);
    unsigned char crcgao = uint8_t(wcrc >> 8);
    data[8] = crcdi;
    data[9] = crcgao;
    qDebug() << "to be write out is ..HHH..." << data.toHex();

    // 更新数据库记录
    if (da.updateCommandRecord(the_id, data)) {
        // 提交事务
        da.db.commit();
        return true;
    }
    else {
        // 回滚事务
        da.db.rollback();
        return false;
    }
}


/*
bool TrackComander::communicationFaMa(QString fama, QString direction) {
    QByteArray ba;
    ba.resize(6);
    if (direction == "Down") {
        ba[0] = 0xfa;
    }
    else {
        // qDebug()<<"commnication fama..... 0xfb";
        ba[0] = 0xfb;
    }
    auto zz = fama.split("|");
    char tempbytearr[9];    // ----> change to 9
    for (int i = 0; i < zz.length(); i++) {
        if (zz[i] == "L") {
            tempbytearr[i] = 0x00;
        }
        else if (zz[i] == "LU") {
            tempbytearr[i] = 0x01;
        }
        else if (zz[i] == "U") {
            tempbytearr[i] = 0x02;
        }
        else if (zz[i] == "UU") {
            tempbytearr[i] = 0x03;
        }
        else if (zz[i] == "U2S") {
            tempbytearr[i] = 0x04;
        }
        else if (zz[i] == "U2") {
            tempbytearr[i] = 0x05;
        }
        else if (zz[i] == "UUS") {
            tempbytearr[i] = 0x06;
        }
        else if (zz[i] == "HB") {
            tempbytearr[i] = 0x07;
        }
        else if (zz[i] == "HU") {
            tempbytearr[i] = 0x08;
        }
        else if (zz[i] == "LU2") {
            tempbytearr[i] = 0x09;
        }
        else if (zz[i] == "ZQ") { //载频切换
            tempbytearr[i] = 0x0a;
        }
        else if (zz[i] == "JC") { //占用检查
            tempbytearr[i] = 0x0b;
        }
        else if (zz[i] == "H") {
            tempbytearr[i] = 0x0c;
        }
        else if (zz[i] == "L2") {
            tempbytearr[i] = 0x0d;
        }
        else if (zz[i] == "L3") {
            tempbytearr[i] = 0x0e;
        }
        else if (zz[i] == "L4") {
            tempbytearr[i] = 0x0f;
        }
        else if (zz[i] == "L5") {
            tempbytearr[i] = 0x10;
        }
        else if (zz[i] == "L6") {
            tempbytearr[i] = 0x11;
        }
        else if (zz[i] == "ZY") {
            tempbytearr[i] = 0x0b;
        }
        else {
            tempbytearr[i] = 0x0b;
        }
    }
    ba[1] = (char)(((tempbytearr[0] << 4) & 0xff) | tempbytearr[1]);
    ba[2] = (char)(((tempbytearr[2] << 4) & 0xff) | tempbytearr[3]);
    ba[3] = (char)(((tempbytearr[4] << 4) & 0xff) | tempbytearr[5]);
    ba[4] = (char)(((tempbytearr[6] << 4) & 0xff) | tempbytearr[7]);
    ba[5] = (char)(((tempbytearr[8] << 4) & 0xff));
    uint16_t wcrc = Utilty::ModbusCRC16(ba);
    unsigned char crcdi = uint8_t(wcrc);
    unsigned char crcgao = uint8_t(wcrc >> 8);
    QByteArray re;
    re.resize(8);
    re[0] = ba[0];
    re[1] = ba[1];
    re[2] = ba[2];
    re[3] = ba[3];
    re[4] = ba[4];
    re[5] = ba[5];
    re[6] = crcdi;
    re[7] = crcgao;
    //先生成唯一ID
    auto the_id = da.insertCommandWithoutContent();


    qDebug() << "Fama driver................." << re.toHex();
    da.insertCommand(re);
    return true;
}

*/



void TrackComander::handlerFama(QString segments, QString famas, QString uplinkornot) {
    this->FaMaHandler(segments, famas, uplinkornot);
}



void TrackComander::InitalPhyFaMa() {
    QMap<QString, QString> segment2fama;
    QString sements = "";
    QString famas = "";
    segment2fama.insert("1107G", "HU");
    segment2fama.insert("1105G", "U");
    segment2fama.insert("1103G", "LU");
    segment2fama.insert("1101G", "L");
    segment2fama.insert("X1LQG", "L2");
    segment2fama.insert("4DG", "JC");
    segment2fama.insert("1G", "HU");
    segment2fama.insert("3-11G", "JC");
    segment2fama.insert("3G", "JC");
    QMap<QString, QString>::iterator it = segment2fama.begin();
    while (it != segment2fama.end()) {
        sements += it.key();
        sements += "|";
        famas += it.value();
        famas += "|";
        it++;
    }
    FaMaHandler(Utilty::removeLastSpiltCharacter(sements), Utilty::removeLastSpiltCharacter(famas), "Down");
    QMap<QString, QString> segment2fama2;
    segment2fama2.insert("S1LQG", "HU");
    segment2fama2.insert("1108G", "HU");
    segment2fama2.insert("1106G", "U");
    segment2fama2.insert("1104G", "LU");
    segment2fama2.insert("1102G", "L");
    segment2fama2.insert("2DG", "JC");
    segment2fama2.insert("IIG", "HU");
    segment2fama2.insert("1-9DG", "JC");
    segment2fama2.insert("4G", "JC");
    sements = "";
    famas = "";
    QMap<QString, QString>::iterator itt = segment2fama2.begin();
    while (itt != segment2fama2.end()) {
        sements += itt.key();
        sements += "|";
        famas += itt.value();
        famas += "|";
        itt++;
    }
    // qDebug()<<"this placesssss.....";
    FaMaHandler(Utilty::removeLastSpiltCharacter(sements), Utilty::removeLastSpiltCharacter(famas), "Up");
}

//界面初始化之后，对界面中的信号机进行驱动
void TrackComander::DrivePhySignalStatus(QMap<QString, QString> signal2color)
{
    QString data1 = "11111";
    QString data2 = "11";
    auto _1107_color = signal2color.value("1107");
    data1 += (_1107_color.contains("Yellow") == true) ? "0" : "1";
    data1 += (_1107_color.contains("Red") == true) ? "0" : "1";
    data1 += (_1107_color.contains("Green") == true) ? "0" : "1";
    auto _1105_color = signal2color.value("1105");
    data2 += (_1105_color.contains("Yellow") == true) ? "0" : "1";
    data2 += (_1105_color.contains("Red") == true) ? "0" : "1";
    data2 += (_1105_color.contains("Green") == true) ? "0" : "1";
    auto _1103_color = signal2color.value("1103");
    data2 += (_1103_color.contains("Yellow") == true) ? "0" : "1";
    data2 += (_1103_color.contains("Red") == true) ? "0" : "1";
    data2 += (_1103_color.contains("Green") == true) ? "0" : "1";
    auto firstbyte = Utilty::convertStrToByte(data1);
    auto secondbyte = Utilty::convertStrToByte(data2);
    QByteArray tempba;
    tempba.resize(7);
    tempba[0] = 0x96;
    tempba[1] = 0xff;
    tempba[2] = 0xff;
    tempba[3] = firstbyte;
    tempba[4] = secondbyte;
    da.db.transaction();
    auto the_id = da.insertCommandWithoutContent();
    qDebug() << "we obtain the command id is...." << the_id;
    QString binaryStr = Utilty::intToQString(the_id);
    auto byte1 = Utilty::convertStrToByte(binaryStr.left(8));
    auto byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
    tempba[1] = byte1;
    tempba[2] = byte2;
    uint16_t wcrc = Utilty::ModbusCRC16(tempba.left(5));
    unsigned char crcdi = uint8_t(wcrc);
    unsigned char crcgao = uint8_t(wcrc >> 8);
    tempba[5] = crcdi;
    tempba[6] = crcgao;
    // 更新数据库记录
    if (da.updateCommandRecord(the_id, tempba)) {
        // 提交事务
        da.db.commit();
    }
    else {
        // 回滚事务
        da.db.rollback();
    }


    data1 = "11";
    data2 = "11";
    auto _1108_color = signal2color.value("1108");
    data1 += (_1108_color.contains("Yellow") == true) ? "0" : "1";
    data1 += (_1108_color.contains("Red") == true) ? "0" : "1";
    data1 += (_1108_color.contains("Green") == true) ? "0" : "1";
    auto _1106_color = signal2color.value("1106");
    data1 += (_1106_color.contains("Yellow") == true) ? "0" : "1";
    data1 += (_1106_color.contains("Red") == true) ? "0" : "1";
    data1 += (_1106_color.contains("Green") == true) ? "0" : "1";
    auto _1104_color = signal2color.value("1104");
    data2 += (_1104_color.contains("Yellow") == true) ? "0" : "1";
    data2 += (_1104_color.contains("Red") == true) ? "0" : "1";
    data2 += (_1104_color.contains("Green") == true) ? "0" : "1";
    auto _1102_color = signal2color.value("1102");
    data2 += (_1102_color.contains("Yellow") == true) ? "0" : "1";
    data2 += (_1102_color.contains("Red") == true) ? "0" : "1";
    data2 += (_1102_color.contains("Green") == true) ? "0" : "1";
    tempba.resize(7);
    tempba[0] = 0x97;
    tempba[1] = 0xff;
    tempba[2] = 0xff;
    tempba[3] = Utilty::convertStrToByte(data1);;
    tempba[4] = Utilty::convertStrToByte(data2);
    da.db.transaction();
    the_id = da.insertCommandWithoutContent();
    qDebug() << "we obtain the command id is...." << the_id;
     binaryStr = Utilty::intToQString(the_id);
     byte1 = Utilty::convertStrToByte(binaryStr.left(8));
     byte2 = Utilty::convertStrToByte(binaryStr.mid(8));

    tempba[1] = byte1;
    tempba[2] = byte2;
    wcrc = Utilty::ModbusCRC16(tempba.left(5));
    crcdi = uint8_t(wcrc);
    crcgao = uint8_t(wcrc >> 8);
    tempba[5] = crcdi;
    tempba[6] = crcgao;
    // 更新数据库记录
    if (da.updateCommandRecord(the_id, tempba)) {
        // 提交事务
        da.db.commit();
    }
    else {
        // 回滚事务
        da.db.rollback();
    }




    data1 = "1";
    data2 = "";
    auto _X4_color = signal2color.value("X4");
    data1 += (_X4_color.contains("Red") == true) ? "0" : "1";
    data1 += (_X4_color.contains("Green") == true) ? "0" : "1";
    auto _X3_color = signal2color.value("X3");
    data1 += (_X3_color.contains("Yellow") == true) ? "0" : "1";
    data1 += (_X3_color.contains("Red") == true) ? "0" : "1";
    data1 += (_X3_color.contains("Green") == true) ? "0" : "1";
    auto _SN_color = signal2color.value("SN");
    data1 += (_SN_color.contains("White") == true) ? "0" : "1";
    data1 += (_SN_color.contains("YellowYellow") == true) ? "0" : "1";
    data2 += (_SN_color.contains("Red") == true) ? "0" : "1";
    data2 += (_SN_color.contains("Green") == true) ? "0" : "1";
    data2 += (_SN_color.contains("Yellow") == true) ? "0" : "1";

    auto _S_color = signal2color.value("S");
    data2 += (_S_color.contains("White") == true) ? "0" : "1";
    data2 += (_S_color.contains("YellowYellow") == true) ? "0" : "1";
    data2 += (_S_color.contains("Red") == true) ? "0" : "1";
    data2 += (_S_color.contains("Green") == true) ? "0" : "1";
    data2 += (_S_color.contains("Yellow") == true) ? "0" : "1";

    tempba.resize(7);
    tempba[0] = 0x95;
    tempba[1] = 0xff;
    tempba[2] = 0xff;
    tempba[3] = Utilty::convertStrToByte(data1);;
    tempba[4] = Utilty::convertStrToByte(data2);
    da.db.transaction();
    the_id = da.insertCommandWithoutContent();
    qDebug() << "we obtain the command id is...." << the_id;
     binaryStr = Utilty::intToQString(the_id);
     byte1 = Utilty::convertStrToByte(binaryStr.left(8));
     byte2 = Utilty::convertStrToByte(binaryStr.mid(8));

    tempba[1] = byte1;
    tempba[2] = byte2;
     wcrc = Utilty::ModbusCRC16(tempba.left(5));
    crcdi = uint8_t(wcrc);
    crcgao = uint8_t(wcrc >> 8);
    tempba[5] = crcdi;
    tempba[6] = crcgao;
    // 更新数据库记录
    if (da.updateCommandRecord(the_id, tempba)) {
        // 提交事务
        da.db.commit();
    }
    else {
        // 回滚事务
        da.db.rollback();
    }


    data1 = "11";
    data2 = "";
    auto _XII_color = signal2color.value("XII");
    data1 += (_XII_color.contains("Red") == true) ? "0" : "1";
    data1 += (_XII_color.contains("Green") == true) ? "0" : "1";
    auto _S4_color = signal2color.value("S4");
    data1 += (_S4_color.contains("White") == true) ? "0" : "1";
    data1 += (_S4_color.contains("Yellow") == true) ? "0" : "1";
    data1 += (_S4_color.contains("Red") == true) ? "0" : "1";
    data1 += (_S4_color.contains("Green") == true) ? "0" : "1";
    auto _S3_color = signal2color.value("S3");
    data2 += (_S3_color.contains("White") == true) ? "0" : "1";
    data2 += (_S3_color.contains("Yellow") == true) ? "0" : "1";
    data2 += (_S3_color.contains("Red") == true) ? "0" : "1";
    data2 += (_S3_color.contains("Green") == true) ? "0" : "1";
    auto _SII_color = signal2color.value("SII");
    data2 += (_SII_color.contains("White") == true) ? "0" : "1";
    data2 += (_SII_color.contains("Yellow") == true) ? "0" : "1";
    data2 += (_SII_color.contains("Red") == true) ? "0" : "1";
    data2 += (_SII_color.contains("Green") == true) ? "0" : "1";
    tempba.resize(7);
    tempba[0] = 0x94;
    tempba[1] = 0xff;
    tempba[2] = 0xff;
    tempba[3] = Utilty::convertStrToByte(data1);;
    tempba[4] = Utilty::convertStrToByte(data2);
    da.db.transaction();
    the_id = da.insertCommandWithoutContent();
    qDebug() << "we obtain the command id is...." << the_id;
     binaryStr = Utilty::intToQString(the_id);
     byte1 = Utilty::convertStrToByte(binaryStr.left(8));
     byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
    tempba[1] = byte1;
    tempba[2] = byte2;
     wcrc = Utilty::ModbusCRC16(tempba.left(5));
    crcdi = uint8_t(wcrc);
    crcgao = uint8_t(wcrc >> 8);
    tempba[5] = crcdi;
    tempba[6] = crcgao;
    // 更新数据库记录
    if (da.updateCommandRecord(the_id, tempba)) {
        // 提交事务
        da.db.commit();
    }
    else {
        // 回滚事务
        da.db.rollback();
    }


    data1 = "11";
    data2 = "";
    auto _SI_color = signal2color.value("SI");
    data1 += (_SI_color.contains("White") == true) ? "0" : "1";
    data1 += (_SI_color.contains("Yellow") == true) ? "0" : "1";
    data1 += (_SI_color.contains("Red") == true) ? "0" : "1";
    data1 += (_SI_color.contains("Green") == true) ? "0" : "1";
    auto _XN_color = signal2color.value("XN");
    data1 += (_XN_color.contains("White") == true) ? "0" : "1";
    data1 += (_XN_color.contains("YellowYellow") == true) ? "0" : "1";
    data2 += (_XN_color.contains("Red") == true) ? "0" : "1";
    data2 += (_XN_color.contains("Green") == true) ? "0" : "1";
    data2 += (_XN_color.contains("Yellow") == true) ? "0" : "1";
    auto _X_color = signal2color.value("X");
    data2 += (_X_color.contains("White") == true) ? "0" : "1";
    data2 += (_X_color.contains("YellowYellow") == true) ? "0" : "1";
    data2 += (_X_color.contains("Red") == true) ? "0" : "1";
    data2 += (_X_color.contains("Green") == true) ? "0" : "1";
    data2 += (_X_color.contains("Yellow") == true) ? "0" : "1";
    tempba.resize(7);
    tempba[0] = 0x93;
    tempba[1] = 0xff;
    tempba[2] = 0xff;
    tempba[3] = Utilty::convertStrToByte(data1);;
    tempba[4] = Utilty::convertStrToByte(data2);

    da.db.transaction();
    the_id = da.insertCommandWithoutContent();
    qDebug() << "we obtain the command id is...." << the_id;
     binaryStr = Utilty::intToQString(the_id);
     byte1 = Utilty::convertStrToByte(binaryStr.left(8));
     byte2 = Utilty::convertStrToByte(binaryStr.mid(8));
    tempba[1] = byte1;
    tempba[2] = byte2;
     wcrc = Utilty::ModbusCRC16(tempba.left(5));
    crcdi = uint8_t(wcrc);
    crcgao = uint8_t(wcrc >> 8);
    tempba[5] = crcdi;
    tempba[6] = crcgao;
    // 更新数据库记录
    if (da.updateCommandRecord(the_id, tempba)) {
        // 提交事务
        da.db.commit();
    }
    else {
        // 回滚事务
        da.db.rollback();
    }

    //老是要处理垃圾情况，也就是

    auto _XI_color = signal2color.value("XI");
    //qDebug()<<".....driverPhiscalSignal...... _XI_color"<<_XI_color;
    auto _1101_color = signal2color.value("1101");
    data1 = "1";
    data1 += (_1101_color.contains("Red") == true) ? "0" : "1";
    data1 += (_1101_color.contains("Yellow") == true) ? "0" : "1";
    data1 += (_1101_color.contains("Green") == true) ? "0" : "1";
    data1 += "1";
    data1 += (_XI_color.contains("Red") == true) ? "0" : "1";
    data1 += (_XI_color.contains("Yellow") == true) ? "0" : "1";
    data1 += (_XI_color.contains("Green") == true) ? "0" : "1";
    tempba.resize(7);   //这里的规定是，沙盘上的XI的红灯必须要使用第一个字节的color2byte_1101_shapan_H， 其它的都是以实物信号机的驱动指令为主
    tempba[0] = 0x91;
    tempba[1] = 0xff;
    tempba[2] = 0xff;
    tempba[3] = Utilty::convertStrToByte(data1);
    tempba[4] = 0xff;

    if (_1101_color.contains("Red") && _XI_color.contains("Red")) {
        tempba[4] = 0xdf;
    }
    if (_1101_color.contains("Red") && _XI_color.contains("Yellow")) {
        tempba[4] = 0xdd;
    }
    if (_1101_color.contains("Red") && _XI_color.contains("Green")) {
        tempba[4] = 0xd1;
    }
    if (_1101_color.contains("Red") && _XI_color.contains("GreenYellow")) {
        tempba[4] = 0xd5;
    }
    if (_1101_color.contains("Yellow") && _XI_color.contains("Red")) {
        tempba[4] = 0xbf;
    }
    if (_1101_color.contains("Yellow") && _XI_color.contains("Yellow")) {
        tempba[4] = 0xbd;
    }
    if (_1101_color.contains("Yellow") && _XI_color.contains("Green")) {
        tempba[4] = 0xb1;
    }
    if (_1101_color.contains("Yellow") && _XI_color.contains("GreenYellow")) {
        tempba[4] = 0xb5;
    }
    if (_1101_color.contains("Green") && _XI_color.contains("Red")) {
        tempba[4] = 0xef;
    }
    if (_1101_color.contains("Green") && _XI_color.contains("Yellow")) {
        tempba[4] = 0xed;
    }
    if (_1101_color.contains("Green") && _XI_color.contains("Green")) {
        tempba[4] = 0xe1;
    }
    if (_1101_color.contains("Green") && _XI_color.contains("GreenYellow")) {
        tempba[4] = 0xe5;
    }
     if (_1101_color.contains("GreenYellow") && _XI_color.contains("Red")) {
        tempba[4] = 0x7f;
    }
    if (_1101_color.contains("GreenYellow") && _XI_color.contains("Yellow")) {
        tempba[4] = 0x7d;
    }
     if (_1101_color.contains("GreenYellow") && _XI_color.contains("Green")) {
        tempba[4] = 0x71;
    }
     if (_1101_color.contains("GreenYellow") && _XI_color.contains("GreenYellow")) {
        tempba[4] = 0x75;
    }

     da.db.transaction();
     the_id = da.insertCommandWithoutContent();
     qDebug() << "we obtain the command id is...." << the_id;
      binaryStr = Utilty::intToQString(the_id);
      byte1 = Utilty::convertStrToByte(binaryStr.left(8));
      byte2 = Utilty::convertStrToByte(binaryStr.mid(8));

     tempba[1] = byte1;
     tempba[2] = byte2;
      wcrc = Utilty::ModbusCRC16(tempba.left(5));
     crcdi = uint8_t(wcrc);
     crcgao = uint8_t(wcrc >> 8);
     tempba[5] = crcdi;
     tempba[6] = crcgao;
     // 更新数据库记录
     if (da.updateCommandRecord(the_id, tempba)) {
         // 提交事务
         da.db.commit();
     }
     else {
         // 回滚事务
         da.db.rollback();
     }
    

}



//对某个转辙机进行定操操作
void TrackComander::handlerSwitchDingcaoOperation(QString switchNumber) {

    auto the_trackline = turnoutInWhichRouteMap.value(switchNumber);
    if (switchNumber == "4") {
        qDebug() << "we are going to here............XXXXXXXXXXXXXXX.." << the_trackline <<"switch num..."<< switchNumber;

        QString trackdirection = da.getCurrentTrackDirectionStatus(the_trackline);
        QByteArray temp_data7;
        temp_data7.resize(3);
        temp_data7[0] = 0x90;
        temp_data7[1] = 0xff;
        //当前轨道正向，且4号道岔期望驱动到定操
        if (trackdirection == "Forward") {
            //我们应该首先查询4号道岔的方向继电器，如果4号道岔是Normal状态，这个时候，我们的驱动指令应该是11011010
            //如果4号道岔是Reverse状态，也要转换过来
            temp_data7[2] = 0xcb;
            auto crcWrappered = Utilty::appendCRC(temp_data7);
            auto commandId = da.insertSwitchOperationCommand(crcWrappered);
            emit newSwitchDirectCommandStored(switchNumber, QString::number(commandId));
            return;
        }
        else {
            temp_data7[2] = 0xeb;
            auto crcWrappered = Utilty::appendCRC(temp_data7);
            auto commandId = da.insertSwitchOperationCommand(crcWrappered);
            emit newSwitchDirectCommandStored(switchNumber, QString::number(commandId));
            return;
        }
    }

    if (switchNumber == "2") {
        QByteArray temp_data7;
        temp_data7.resize(3);
        temp_data7[0] = 0x92;
        temp_data7[2] = 0xff;
        temp_data7[1] = 0xef;
        auto crcWrappered = Utilty::appendCRC(temp_data7);
        auto commandId = da.insertSwitchOperationCommand(crcWrappered);
        emit newSwitchDirectCommandStored(switchNumber, QString::number(commandId));
        return;
    }

    if (switchNumber == "9") {
        QByteArray temp_data7;
        temp_data7.resize(3);
        temp_data7[0] = 0x92;
        temp_data7[2] = 0xff;
        temp_data7[1] = 0xfe;
        auto crcWrappered = Utilty::appendCRC(temp_data7);
        auto commandId = da.insertSwitchOperationCommand(crcWrappered);
        emit newSwitchDirectCommandStored(switchNumber, QString::number(commandId));
        return;
    }


    if (switchNumber == "11") {
        QByteArray temp_data7;
        temp_data7.resize(3);
        temp_data7[0] = 0x92;
        temp_data7[2] = 0xff;
        temp_data7[1] = 0xfb;
        auto crcWrappered = Utilty::appendCRC(temp_data7);
        auto commandId = da.insertSwitchOperationCommand(crcWrappered);
        emit newSwitchDirectCommandStored(switchNumber, QString::number(commandId));
        return;
    }

    if (switchNumber == "5" || switchNumber == "7") {
        QByteArray temp_data7;
        temp_data7.resize(3);
        temp_data7[0] = 0x92;
        temp_data7[2] = 0xaf;
        temp_data7[1] = 0xff;
        auto crcWrappered = Utilty::appendCRC(temp_data7);
        auto commandId = da.insertSwitchOperationCommand(crcWrappered);
        emit newSwitchDirectCommandStored(switchNumber, QString::number(commandId));
        return;
    }

    if (switchNumber == "1" || switchNumber == "3") {
        QByteArray temp_data7;
        temp_data7.resize(3);
        temp_data7[0] = 0x92;
        temp_data7[2] = 0xfa;
        temp_data7[1] = 0xff;
        auto crcWrappered = Utilty::appendCRC(temp_data7);
        auto commandId = da.insertSwitchOperationCommand(crcWrappered);
        emit newSwitchDirectCommandStored(switchNumber, QString::number(commandId));
        return;
    }
    
}



void TrackComander::handlerSwitchReverseOperation(QString switchNumber) {

    auto the_trackline = turnoutInWhichRouteMap.value(switchNumber);
    if (switchNumber == "4") {
        qDebug() << "we are going to here............XXXXXXXXXXXXXXX.." << the_trackline << "switch num..." << switchNumber;

        QString trackdirection = da.getCurrentTrackDirectionStatus(the_trackline);
        QByteArray temp_data7;
        temp_data7.resize(3);
        temp_data7[0] = 0x90;
        temp_data7[1] = 0xff;
        //当前轨道正向，且4号道岔期望驱动到反操
        if (trackdirection == "Forward") {
            //我们应该首先查询4号道岔的方向继电器，如果4号道岔是Normal状态，这个时候，我们的驱动指令应该是11100101
            temp_data7[2] = 0xc5;
            auto crcWrappered = Utilty::appendCRC(temp_data7);
            auto commandId = da.insertSwitchOperationCommand(crcWrappered);
            emit newSwitchReverseCommandStored(switchNumber, QString::number(commandId));
            return;
        }
        else {          //其实就是根据列车前进方向，要注意道岔的定反操只不过是控制4#FCJ	4#DCJ	4#FC	4#DC
            temp_data7[2] = 0xe5;
            auto crcWrappered = Utilty::appendCRC(temp_data7);
            auto commandId = da.insertSwitchOperationCommand(crcWrappered);
            emit newSwitchReverseCommandStored(switchNumber, QString::number(commandId));
            return;
        }
    }

    if (switchNumber == "2") {
        QByteArray temp_data7;
        temp_data7.resize(3);
        temp_data7[0] = 0x92;
        temp_data7[2] = 0xff;
        temp_data7[1] = 0xdf;
        auto crcWrappered = Utilty::appendCRC(temp_data7);
        auto commandId = da.insertSwitchOperationCommand(crcWrappered);
        emit newSwitchReverseCommandStored(switchNumber, QString::number(commandId));
        return;
    }

    if (switchNumber == "9") {
        QByteArray temp_data7;
        temp_data7.resize(3);
        temp_data7[0] = 0x92;
        temp_data7[2] = 0xff;
        temp_data7[1] = 0xfd;
        auto crcWrappered = Utilty::appendCRC(temp_data7);
        auto commandId = da.insertSwitchOperationCommand(crcWrappered);
        emit newSwitchReverseCommandStored(switchNumber, QString::number(commandId));
        return;
    }


    if (switchNumber == "11") {
        QByteArray temp_data7;
        temp_data7.resize(3);
        temp_data7[0] = 0x92;
        temp_data7[2] = 0xff;
        temp_data7[1] = 0xf7;
        auto crcWrappered = Utilty::appendCRC(temp_data7);
        auto commandId = da.insertSwitchOperationCommand(crcWrappered);
        emit newSwitchReverseCommandStored(switchNumber, QString::number(commandId));
        return;
    }

    if (switchNumber == "5" || switchNumber == "7") {
        QByteArray temp_data7;
        temp_data7.resize(3);
        temp_data7[0] = 0x92;
        temp_data7[2] = 0x5f;
        temp_data7[1] = 0xff;
        auto crcWrappered = Utilty::appendCRC(temp_data7);
        auto commandId = da.insertSwitchOperationCommand(crcWrappered);
        emit newSwitchReverseCommandStored(switchNumber, QString::number(commandId));
        return;
    }

    if (switchNumber == "1" || switchNumber == "3") {
        QByteArray temp_data7;
        temp_data7.resize(3);
        temp_data7[0] = 0x92;
        temp_data7[2] = 0xf5;
        temp_data7[1] = 0xff;
        auto crcWrappered = Utilty::appendCRC(temp_data7);
        auto commandId = da.insertSwitchOperationCommand(crcWrappered);
        emit newSwitchReverseCommandStored(switchNumber, QString::number(commandId));
        return;
    }


}


void TrackComander::DriverSpecifiedSignalToSpecifiedColor(QString signalname, QString wanteclor) {
    auto the_Status = wanteclor.split("|").first().trimmed() + wanteclor.split("|").last().trimmed();
    this->driverOnePhysignalToSpecificColor(signalname, the_Status);
    return;
}


