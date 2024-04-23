#include "TracktologyBuilder.h"
#include "DataAccessLayer.h"
#include "Utilty.h"

TracktologyBuilder::TracktologyBuilder()
{

}

void TracktologyBuilder::updateFakeSwitchStatus(QByteArray ba)              //更新虚拟的转辙机状态
{

    rawVitualSwitchsStatus["1|DB"] = ba[1] & 0x01;
    rawVitualSwitchsStatus["1|FB"] = (ba[1] & 0x02) >> 1;
    rawVitualSwitchsStatus["3|DB"] = (ba[1] & 0x04) >> 2;
    rawVitualSwitchsStatus["3|FB"] = (ba[1] & 0x08) >> 3;
    rawVitualSwitchsStatus["5|DB"] = (ba[1] & 0x10) >> 4;
    rawVitualSwitchsStatus["5|FB"] = (ba[1] & 0x20) >> 5;
    rawVitualSwitchsStatus["7|DB"] = (ba[1] & 0x40) >> 6;
    rawVitualSwitchsStatus["7|FB"] = (ba[1] & 0x80) >> 7;
    rawVitualSwitchsStatus["9|DB"] = ba[0] & 0x01;
    rawVitualSwitchsStatus["9|FB"] = (ba[0] & 0x02) >> 1;
    rawVitualSwitchsStatus["11|DB"] = (ba[0] & 0x04) >> 2;
    rawVitualSwitchsStatus["11|FB"] = (ba[0] & 0x08) >> 3;
    rawVitualSwitchsStatus["2|DB"] = (ba[0] & 0x10) >> 4;
    rawVitualSwitchsStatus["2|FB"] = (ba[0] & 0x20) >> 5;
    qDebug()<<"我们获得到了非实物转辙机信息..........."<< rawVitualSwitchsStatus["1|DB"] <<
        rawVitualSwitchsStatus["1|FB"] <<
        rawVitualSwitchsStatus["3|DB"] <<
        rawVitualSwitchsStatus["3|FB"] <<
        rawVitualSwitchsStatus["5|DB"] <<
        rawVitualSwitchsStatus["5|FB"] <<
        rawVitualSwitchsStatus["7|DB"] <<
        rawVitualSwitchsStatus["7|FB"] <<
        rawVitualSwitchsStatus["9|DB"] <<
        rawVitualSwitchsStatus["9|FB"] <<
        rawVitualSwitchsStatus["11|DB"] <<
        rawVitualSwitchsStatus["11|FB"] <<
        rawVitualSwitchsStatus["2|DB"] <<
        rawVitualSwitchsStatus["2|FB"];

    if (rawVitualSwitchsStatus["1|DB"] == 0x00 && rawVitualSwitchsStatus["1|FB"] == 0x01) {
        da.updateTurnoutStatus("1", "Normal");
        emit turoutStatusUpdated("1", "Normal");
    }
    else if (rawVitualSwitchsStatus["1|DB"] == 0x01 && rawVitualSwitchsStatus["1|FB"] == 0x00) {
        da.updateTurnoutStatus("1", "Reverse");
        emit turoutStatusUpdated("1", "Reverse");
    }
    else {
        da.updateTurnoutStatus("1", "Lost");
        emit turoutStatusUpdated("1", "Lost");
    }

    if (rawVitualSwitchsStatus["3|DB"] == 0x00 && rawVitualSwitchsStatus["3|FB"] == 0x01) {
        da.updateTurnoutStatus("3", "Normal");
        emit turoutStatusUpdated("3", "Normal");
    }
    else if (rawVitualSwitchsStatus["3|DB"] == 0x01 && rawVitualSwitchsStatus["3|FB"] == 0x00) {
        da.updateTurnoutStatus("3", "Reverse");
        emit turoutStatusUpdated("3", "Reverse");
    }
    else {
        da.updateTurnoutStatus("3", "Lost");
        emit turoutStatusUpdated("3", "Lost");
    }
 
    if (rawVitualSwitchsStatus["5|DB"] == 0x00 && rawVitualSwitchsStatus["5|FB"] == 0x01) {
        da.updateTurnoutStatus("5", "Normal");
        emit turoutStatusUpdated("5", "Normal");
    }
    else if (rawVitualSwitchsStatus["5|DB"] == 0x01 && rawVitualSwitchsStatus["5|DB"] == 0x00) {
        da.updateTurnoutStatus("5", "Reverse");
        emit turoutStatusUpdated("5", "Reverse");
    }
    else {
        da.updateTurnoutStatus("5", "Lost");
        emit turoutStatusUpdated("5", "Lost");
    }

    if (rawVitualSwitchsStatus["7|DB"] == 0x00 && rawVitualSwitchsStatus["7|FB"] == 0x01) {
        da.updateTurnoutStatus("7", "Normal");
        emit turoutStatusUpdated("7", "Normal");
    }
    else if (rawVitualSwitchsStatus["7|DB"] == 0x01 && rawVitualSwitchsStatus["7|DB"] == 0x00) {
        da.updateTurnoutStatus("7", "Reverse");
        emit turoutStatusUpdated("7", "Reverse");
    }
    else {
        da.updateTurnoutStatus("7", "Lost");
        emit turoutStatusUpdated("7", "Lost");
    }


    if (rawVitualSwitchsStatus["9|DB"] == 0x00 && rawVitualSwitchsStatus["9|FB"] == 0x01) {
        da.updateTurnoutStatus("9", "Normal");
        emit turoutStatusUpdated("9", "Normal");
    }
    else if (rawVitualSwitchsStatus["9|DB"] == 0x01 && rawVitualSwitchsStatus["9|DB"] == 0x00) {
        da.updateTurnoutStatus("9", "Reverse");
        emit turoutStatusUpdated("9", "Reverse");
    }
    else {
        da.updateTurnoutStatus("9", "Lost");
        emit turoutStatusUpdated("9", "Lost");
    }

    if (rawVitualSwitchsStatus["11|DB"] == 0x00 && rawVitualSwitchsStatus["11|FB"] == 0x01) {
        da.updateTurnoutStatus("11", "Normal");
        emit turoutStatusUpdated("11", "Normal");
    }
    else if (rawVitualSwitchsStatus["11|DB"] == 0x01 && rawVitualSwitchsStatus["11|DB"] == 0x00) {
        da.updateTurnoutStatus("11", "Reverse");
        emit turoutStatusUpdated("11", "Reverse");
    }
    else {
        da.updateTurnoutStatus("11", "Lost");
        emit turoutStatusUpdated("11", "Lost");
    }
    if (rawVitualSwitchsStatus["2|DB"] == 0x00 && rawVitualSwitchsStatus["2|FB"] == 0x01) {
        da.updateTurnoutStatus("2", "Normal");
        emit turoutStatusUpdated("2", "Normal");
    }
    else if (rawVitualSwitchsStatus["2|DB"] == 0x01 && rawVitualSwitchsStatus["2|DB"] == 0x00) {
        da.updateTurnoutStatus("2", "Reverse");
        emit turoutStatusUpdated("2", "Reverse");
    }
    else {
        da.updateTurnoutStatus("2", "Lost");
        emit turoutStatusUpdated("2", "Lost");
    }
}




void TracktologyBuilder::updateFakeSignalStatus(QByteArray ba,int classno){
    if (ba.length()!=2){
        return;
    }
    if (classno==1){   //0xb3
        rawVirtualSignalsStatus["XN|UU"] = ba[0] & 0x01;
        rawVirtualSignalsStatus["XN|B"] = (ba[0] & 0x02)>>1;
        rawVirtualSignalsStatus["SI|L"] = (ba[0] & 0x04)>>2;
        rawVirtualSignalsStatus["SI|H"] =  (ba[0] & 0x08)>>3;
        rawVirtualSignalsStatus["SI|U"] =  (ba[0] & 0x10)>>4;
        rawVirtualSignalsStatus["SI|B"] = (ba[0] & 0x20)>>5;
        rawVirtualSignalsStatus["X|U"] =   ba[1] & 0x01;
        rawVirtualSignalsStatus["X|L"] =  (ba[1] & 0x02)>>1;
        rawVirtualSignalsStatus["X|H"] =  (ba[1] & 0x04)>>2;
        rawVirtualSignalsStatus["X|UU"] = (ba[1] & 0x08)>>3;
        rawVirtualSignalsStatus["X|B"] = (ba[1] & 0x10)>>4;
        rawVirtualSignalsStatus["XN|U"] = (ba[1] & 0x20)>>5;
        rawVirtualSignalsStatus["XN|L"] = (ba[1] & 0x40)>>6;
        rawVirtualSignalsStatus["XN|H"] = (ba[1] & 0x80)>>7;
        
        qDebug() << "我们得到的数据是...." << rawVirtualSignalsStatus["XN|UU"] <<
            rawVirtualSignalsStatus["XN|B"] <<
            rawVirtualSignalsStatus["SI|L"] <<
            rawVirtualSignalsStatus["SI|H"] <<
            rawVirtualSignalsStatus["SI|U"] <<
            rawVirtualSignalsStatus["SI|B"] <<
            rawVirtualSignalsStatus["X|U"] <<
            rawVirtualSignalsStatus["X|L"] <<
            rawVirtualSignalsStatus["X|H"] <<
            rawVirtualSignalsStatus["X|UU"] <<
            rawVirtualSignalsStatus["X|B"] <<
            rawVirtualSignalsStatus["XN|U"] <<
            rawVirtualSignalsStatus["XN|L"] <<
            rawVirtualSignalsStatus["XN|H"];

        unsigned char L, H, U, B, UU;
        L = rawVirtualSignalsStatus["SI|L"];
        H = rawVirtualSignalsStatus["SI|H"];
        U = rawVirtualSignalsStatus["SI|U"];
        B = rawVirtualSignalsStatus["SI|B"];
        auto bishiclolor = Utilty::getSignalColorFrom4Combination(L, H, U, B);
        qDebug() << "SI得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U << B;
        da.updateOrInsertSignalStatus("SI", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("SI", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());

        UU = rawVirtualSignalsStatus["XN|UU"];
        B =  rawVirtualSignalsStatus["XN|B"];
        U =  rawVirtualSignalsStatus["XN|U"];
        L =  rawVirtualSignalsStatus["XN|L"];
        H =  rawVirtualSignalsStatus["XN|H"];

        bishiclolor = Utilty::getSignalColorFrom5Combination(L, H, U, B,UU);
        qDebug() << "XN得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U << B;
        da.updateOrInsertSignalStatus("XN", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("XN", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());


        UU = rawVirtualSignalsStatus["X|UU"];
        B = rawVirtualSignalsStatus["X|B"];
        U = rawVirtualSignalsStatus["X|U"];
        L = rawVirtualSignalsStatus["X|L"];
        H = rawVirtualSignalsStatus["X|H"];

        bishiclolor = Utilty::getSignalColorFrom5Combination(L, H, U, B, UU);
        qDebug() << "X得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U << B;
        da.updateOrInsertSignalStatus("X", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("X", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());


    }else if (classno==2){ //0Xb4
        rawVirtualSignalsStatus["S4|L"] = ba[0] & 0x01;
        rawVirtualSignalsStatus["S4|H"] = (ba[0] & 0x02)>>1;
        rawVirtualSignalsStatus["S4|U"] = (ba[0] & 0x04)>>2;
        rawVirtualSignalsStatus["S4|B"] = (ba[0] & 0x08)>>3;
        rawVirtualSignalsStatus["XII|L"] = (ba[0] & 0x10)>>4;
        rawVirtualSignalsStatus["XII|H"] = (ba[0] & 0x20)>>5;
        rawVirtualSignalsStatus["SII|L"] = ba[1] & 0x01;
        rawVirtualSignalsStatus["SII|H"] = (ba[1] & 0x02)>>1;
        rawVirtualSignalsStatus["SII|U"] = (ba[1] & 0x04)>>2;
        rawVirtualSignalsStatus["SII|B"] = (ba[1] & 0x08)>>3;
        rawVirtualSignalsStatus["S3|L"] = (ba[1] & 0x10)>>4;
        rawVirtualSignalsStatus["S3|H"] = (ba[1] & 0x20)>>5;
        rawVirtualSignalsStatus["S3|U"] = (ba[1] & 0x40)>>6;
        rawVirtualSignalsStatus["S3|B"] = (ba[1] & 0x80)>>7;

        unsigned char L, H, U, B, UU;
        L = rawVirtualSignalsStatus["S4|L"];
        H = rawVirtualSignalsStatus["S4|H"];
        U = rawVirtualSignalsStatus["S4|U"];
        B = rawVirtualSignalsStatus["S4|B"];
        auto bishiclolor = Utilty::getSignalColorFrom4Combination(L, H, U, B);
        qDebug() << "S4得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U << B;
        da.updateOrInsertSignalStatus("S4", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("S4", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());


        L = rawVirtualSignalsStatus["S3|L"];
        H = rawVirtualSignalsStatus["S3|H"];
        U = rawVirtualSignalsStatus["S3|U"];
        B = rawVirtualSignalsStatus["S3|B"];
        bishiclolor = Utilty::getSignalColorFrom4Combination(L, H, U, B);
        qDebug() << "S3得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U << B;
        da.updateOrInsertSignalStatus("S3", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("S3", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());



        L = rawVirtualSignalsStatus["SII|L"];
        H = rawVirtualSignalsStatus["SII|H"];
        U = rawVirtualSignalsStatus["SII|U"];
        B = rawVirtualSignalsStatus["SII|B"];
        bishiclolor = Utilty::getSignalColorFrom4Combination(L, H, U, B);
        qDebug() << "SII得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U << B;
        da.updateOrInsertSignalStatus("SII", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("SII", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());


        if (rawVirtualSignalsStatus["XII|L"] == 0 && rawVirtualSignalsStatus["XII|H"] == 1) {
            da.updateOrInsertSignalStatus("XII", "Green","Black");
            emit trackSignalStatusUpdated("XII",  "Green|Black");
        }else if (rawVirtualSignalsStatus["XII|L"] == 1 && rawVirtualSignalsStatus["XII|H"] == 0) {
            da.updateOrInsertSignalStatus("XII", "Red", "Black");
            emit trackSignalStatusUpdated("XII", "Red|Black");
        }
        else {
            da.updateOrInsertSignalStatus("XII", "Red", "Black");
            emit trackSignalStatusUpdated("XII", "Red|Black");
        }

 
    }
    else if (classno==3){ //0Xb5
        rawVirtualSignalsStatus["SN|UU"] = ba[0] & 0x01;
        rawVirtualSignalsStatus["SN|B"] = (ba[0] & 0x02)>>1;
        rawVirtualSignalsStatus["X3|L"] = (ba[0] & 0x04)>>2;
        rawVirtualSignalsStatus["X3|H"] = (ba[0] & 0x08)>>3;
        rawVirtualSignalsStatus["X3|U"] = (ba[0] & 0x10)>>4;
        rawVirtualSignalsStatus["X4|L"] = (ba[0] & 0x20)>>5;
        rawVirtualSignalsStatus["X4|H"] = (ba[0] & 0x40)>>6;
        rawVirtualSignalsStatus["S|U"] = ba[1] & 0x01;
        rawVirtualSignalsStatus["S|L"] = (ba[1] & 0x02)>>1;
        rawVirtualSignalsStatus["S|H"] = (ba[1] & 0x04)>>2;
        rawVirtualSignalsStatus["S|UU"] = (ba[1] & 0x08)>>3;
        rawVirtualSignalsStatus["S|B"] = (ba[1] & 0x10)>>4;
        rawVirtualSignalsStatus["SN|U"] = (ba[1] & 0x20)>>5;
        rawVirtualSignalsStatus["SN|L"] = (ba[1] & 0x40)>>6;
        rawVirtualSignalsStatus["SN|H"] = (ba[1] & 0x80)>>7;


        unsigned char L, H, U, B, UU;

        UU = rawVirtualSignalsStatus["SN|UU"];
        B = rawVirtualSignalsStatus["SN|B"];
        U = rawVirtualSignalsStatus["SN|U"];
        L = rawVirtualSignalsStatus["SN|L"];
        H = rawVirtualSignalsStatus["SN|H"];

        auto bishiclolor = Utilty::getSignalColorFrom5Combination(L, H, U, B, UU);
        qDebug() << "SN得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U << B;
        da.updateOrInsertSignalStatus("SN", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("SN", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());



        UU = rawVirtualSignalsStatus["S|UU"];
        B = rawVirtualSignalsStatus["S|B"];
        U = rawVirtualSignalsStatus["S|U"];
        L = rawVirtualSignalsStatus["S|L"];
        H = rawVirtualSignalsStatus["S|H"];

        bishiclolor = Utilty::getSignalColorFrom5Combination(L, H, U, B, UU);
        qDebug() << "S得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U << B;
        da.updateOrInsertSignalStatus("S", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("S", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());


        U = rawVirtualSignalsStatus["X3|U"];
        L = rawVirtualSignalsStatus["X3|L"];
        H = rawVirtualSignalsStatus["X3|H"];

        bishiclolor = Utilty::getSignalColorFrom3Combination(L, H, U);
        qDebug() << "X3得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U << B;
        da.updateOrInsertSignalStatus("X3", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("X3", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());



        if (rawVirtualSignalsStatus["X4|L"] == 0 && rawVirtualSignalsStatus["X4|H"] == 1) {
            da.updateOrInsertSignalStatus("X4", "Green", "Black");
            emit trackSignalStatusUpdated("X4", "Green|Black");
        }
        else if (rawVirtualSignalsStatus["X4|L"] == 1 && rawVirtualSignalsStatus["X4|H"] == 0) {
            da.updateOrInsertSignalStatus("X4", "Red", "Black");
            emit trackSignalStatusUpdated("X4", "Red|Black");
        }
        else {
            da.updateOrInsertSignalStatus("X4", "Red", "Black");
            emit trackSignalStatusUpdated("X4", "Red|Black");
        }

    }
    else if (classno==4){ //0Xb6
        rawVirtualSignalsStatus["1107|L"] = ba[0] & 0x01;
        rawVirtualSignalsStatus["1107|H"] = (ba[0] & 0x02)>>1;
        rawVirtualSignalsStatus["1107|U"] = (ba[0] & 0x04)>>2;
        rawVirtualSignalsStatus["1103|L"] = ba[1] & 0x01;
        rawVirtualSignalsStatus["1103|H"] = (ba[1] & 0x02)>>1;
        rawVirtualSignalsStatus["1103|U"] = (ba[1] & 0x04)>>2;
        rawVirtualSignalsStatus["1105|L"] = (ba[1] & 0x08)>>3;
        rawVirtualSignalsStatus["1105|H"] = (ba[1] & 0x10)>>4;
        rawVirtualSignalsStatus["1105|U"] = (ba[1] & 0x20)>>5;

        unsigned char L, H, U;
        U = rawVirtualSignalsStatus["1107|U"];
        L = rawVirtualSignalsStatus["1107|L"];
        H = rawVirtualSignalsStatus["1107|H"];

        auto bishiclolor = Utilty::getSignalColorFrom3Combination(L, H, U);
        da.updateOrInsertSignalStatus("1107", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("1107", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());

        U = rawVirtualSignalsStatus["1103|U"];
        L = rawVirtualSignalsStatus["1103|L"];
        H = rawVirtualSignalsStatus["1103|H"];

        bishiclolor = Utilty::getSignalColorFrom3Combination(L, H, U);
        da.updateOrInsertSignalStatus("1103", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("1103", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());

        U = rawVirtualSignalsStatus["1105|U"];
        L = rawVirtualSignalsStatus["1105|L"];
        H = rawVirtualSignalsStatus["1105|H"];

        bishiclolor = Utilty::getSignalColorFrom3Combination(L, H, U);
        qDebug() << "1105得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U ;
        da.updateOrInsertSignalStatus("1105", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("1105", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());
       

    }
    else if (classno==5){//0Xb7
        rawVirtualSignalsStatus["1106|L"] = ba[0] & 0x01;
        rawVirtualSignalsStatus["1106|H"] = (ba[0] & 0x02)>>1;
        rawVirtualSignalsStatus["1106|U"] = (ba[0] & 0x04)>>2;
        rawVirtualSignalsStatus["1108|L"] = (ba[0] & 0x08)>>3;
        rawVirtualSignalsStatus["1108|H"] = (ba[0] & 0x10)>>4;
        rawVirtualSignalsStatus["1108|U"] = (ba[0] & 0x20)>>5;
        rawVirtualSignalsStatus["1102|L"] = ba[1] & 0x01;
        rawVirtualSignalsStatus["1102|H"] = (ba[1] & 0x02)>>1;
        rawVirtualSignalsStatus["1102|U"] = (ba[1] & 0x04)>>2;
        rawVirtualSignalsStatus["1104|L"] = (ba[1] & 0x08)>>3;
        rawVirtualSignalsStatus["1104|H"] = (ba[1] & 0x10)>>4;
        rawVirtualSignalsStatus["1104|U"] = (ba[1] & 0x20)>>5;
        
        unsigned char L, H, U;
        U = rawVirtualSignalsStatus["1106|U"];
        L = rawVirtualSignalsStatus["1106|L"];
        H = rawVirtualSignalsStatus["1106|H"];

        auto bishiclolor = Utilty::getSignalColorFrom3Combination(L, H, U);
        qDebug() << "1106得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U ;
        da.updateOrInsertSignalStatus("1106", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("1106", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());

        U = rawVirtualSignalsStatus["1108|U"];
        L = rawVirtualSignalsStatus["1108|L"];
        H = rawVirtualSignalsStatus["1108|H"];

        bishiclolor = Utilty::getSignalColorFrom3Combination(L, H, U);
        qDebug() << "1108得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U;
        da.updateOrInsertSignalStatus("1108", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("1108", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());

        U = rawVirtualSignalsStatus["1102|U"];
        L = rawVirtualSignalsStatus["1102|L"];
        H = rawVirtualSignalsStatus["1102|H"];

        bishiclolor = Utilty::getSignalColorFrom3Combination(L, H, U);
        qDebug() << "1102得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U;
        da.updateOrInsertSignalStatus("1102", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("1102", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());


        U = rawVirtualSignalsStatus["1104|U"];
        L = rawVirtualSignalsStatus["1104|L"];
        H = rawVirtualSignalsStatus["1104|H"];

        bishiclolor = Utilty::getSignalColorFrom3Combination(L, H, U);
        qDebug() << "1104得到正确的颜色了么？？....." << bishiclolor << "from..." << L << H << U;
        da.updateOrInsertSignalStatus("1104", bishiclolor.split('|').first(), bishiclolor.split('|').last());
        emit trackSignalStatusUpdated("1104", bishiclolor.split('|').first() + "|" + bishiclolor.split('|').last());

    }
    else{
        return;
    }
}

//QString TracktologyBuilder::InspectFakeSingalStausFromRawData(QString signal_name){
//
//    QString color1="";
//    QString color2="";
//    //这两个就只有L和H两个色灯，要么是绿，要么是红，其它的做特殊处理
//    if(signal_name=="XII" || signal_name=="X4"){
//
//        if (rawVirtualSignalsStatus.contains(signal_name + "|H") && rawVirtualSignalsStatus[signal_name + "|H"] == 0) {
//
//            color1="Red";
//            color2="Black";
//        }else{
//
//            if (rawVirtualSignalsStatus.contains(signal_name + "|H") && rawVirtualSignalsStatus[signal_name + "|H"] == 0) {
//                color1="Green";
//                color2="Black";
//            }else{
//                color1="Black";
//                color2="Black";
//            }
//
//        }
//
//    }
//    else if (signal_name == "SI" || signal_name == "XN") {
//        unsigned char L, H, U,  B;
//        L = rawVirtualSignalsStatus[signal_name+"|L"];
//        H = rawVirtualSignalsStatus[signal_name + "|H"];
//        U = rawVirtualSignalsStatus[signal_name + "|U"];
//        B = rawVirtualSignalsStatus[signal_name + "|B"];
//        auto bishiclolor = Utilty::getSignalColorFromCombination(L, H, U, B);
//        qDebug() << "SI得到正确的颜色了么？？....." << bishiclolor<< "from..." << L << H << U << B;
//        return bishiclolor;
//    }
//    else{
//
//        getFakeSignalColors(rawVirtualSignalsStatus,signal_name,color1,color2);
//    }
//    return color1+"|"+color2;
//}


void TracktologyBuilder::updateRealSignalStatus(QByteArray ba){
    if(ba.length()!=2){
        return;
    }
    qDebug() << "our TracktologyBuilder get data....PPPPPPPPPPPP" << ba.toHex();
    //LXJ无效点红灯，LXJ有效LJ有效点绿灯，LXJ有效LJ无效LUJ有效点黄灯，LXJ与LUJ有效点绿黄灯
    this->rawPhySignalsStatus["XILXJ"] = ba[1]&0x01;
    this->rawPhySignalsStatus["XILJ"] = (ba[1]&0x02)>>1;
    this->rawPhySignalsStatus["XILUJ"] =  (ba[1]&0x04)>>2;
    this->rawPhySignalsStatus["XIDJ"] =  (ba[1]&0x08)>>3;
    this->rawPhySignalsStatus["1101DJ"] = (ba[1]&0x10)>>4;
    this->rawPhySignalsStatus["11012DJ"] =(ba[1]&0x20)>>5;
    this->rawPhySignalsStatus["1101LJ"] =(ba[1]&0x40)>>6;
    this->rawPhySignalsStatus["1101HJ"] =(ba[1]&0x80)>>7;
    this->rawPhySignalsStatus["1101UJ"] = ba[0]&0x01;
    this->rawPhySignalsStatus["1101LUJ"] =(ba[0]&0x02)>>1;
    this->rawPhySignalsStatus["XIL"] =(ba[0] & 0x04)>>2;
    this->rawPhySignalsStatus["XIU"] =(ba[0] & 0x08)>>3;
    this->rawPhySignalsStatus["X1H"] =(ba[0] & 0x10)>>4;
    this->rawPhySignalsStatus["1101L"] =(ba[0] & 0x20)>>5;
    this->rawPhySignalsStatus["1101U"] =(ba[0] & 0x04)>>6;
    this->rawPhySignalsStatus["1101H"] =(ba[0] & 0x08)>>7;
    //直接基于此得到XI和1101信号机颜色,添加到数据库的表中
    if(rawPhySignalsStatus["XIDJ"]== 0x00
            &&  rawPhySignalsStatus["XILUJ"]==0x01
            &&  rawPhySignalsStatus["XILJ"]==0x01
            &&  rawPhySignalsStatus["XILXJ"]==0x01
            ){
        QString light1="Red";
        QString light2="Black";
        da.updateOrInsertSignalStatus("XI",light1,light2);
        emit trackSignalStatusUpdated("XI", light1 +"|"+ light2);
           
    }

    //这个地方是对的
    if(rawPhySignalsStatus["XIDJ"]== 0x00
            &&  rawPhySignalsStatus["XILUJ"]==0x01
            &&  rawPhySignalsStatus["XILJ"]==0x01
            &&  rawPhySignalsStatus["XILXJ"]==0x00
            ){
        QString light1="Yellow";
        QString light2="Black";
        da.updateOrInsertSignalStatus("XI",light1,light2);
        emit trackSignalStatusUpdated("XI", light1 + "|" + light2);
    }

    if(rawPhySignalsStatus["XIDJ"]== 0x00
            &&  rawPhySignalsStatus["XILUJ"]==0x00
            &&  rawPhySignalsStatus["XILJ"]==0x01
            &&  rawPhySignalsStatus["XILXJ"]==0x00
            ){
        QString light1="Green";
        QString light2="Yellow";
        da.updateOrInsertSignalStatus("XI",light1,light2);
        emit trackSignalStatusUpdated("XI", light1 + "|" + light2);
    }

    if(rawPhySignalsStatus["XIDJ"]== 0x00
            &&  rawPhySignalsStatus["XILUJ"]==0x01
            &&  rawPhySignalsStatus["XILJ"]==0x00
            &&  rawPhySignalsStatus["XILXJ"]==0x00
            ){

        QString light1="Green";
        QString light2="Black";
        da.updateOrInsertSignalStatus("XI",light1,light2);
        emit trackSignalStatusUpdated("XI", light1 + "|" + light2);
    }

    if(this->rawPhySignalsStatus["1101DJ"]==0x00 &&
            this->rawPhySignalsStatus["1101HJ"]==0x00
            ){

        QString light1="Red";
        QString light2="Black";
        da.updateOrInsertSignalStatus("1101",light1,light2);
        emit trackSignalStatusUpdated("1101", light1 + "|" + light2);
    }
    if(this->rawPhySignalsStatus["1101DJ"]==0x00 &&
            this->rawPhySignalsStatus["1101LJ"]==0x00
            ){

        QString light1="Green";
        QString light2="Black";
        da.updateOrInsertSignalStatus("1101",light1,light2);
        emit trackSignalStatusUpdated("1101", light1 + "|" + light2);
    }
    if(this->rawPhySignalsStatus["11012DJ"]==0x00 &&
            this->rawPhySignalsStatus["1101UJ"]==0x00
            ){

        QString light1="Yellow";
        QString light2="Black";
        da.updateOrInsertSignalStatus("1101",light1,light2);
        emit trackSignalStatusUpdated("1101", light1 + "|" + light2);

    }
    if(this->rawPhySignalsStatus["1101DJ"]==0x00 &&
            this->rawPhySignalsStatus["1101LUJ"]==0x00
            ){
        QString light1="Green";
        QString light2="Yellow";
        da.updateOrInsertSignalStatus("1101",light1,light2);
        emit trackSignalStatusUpdated("1101", light1 + "|" + light2);
    }
}


void TracktologyBuilder::getFakeSignalColors(QMap<QString, unsigned char> &rawSignalsStatus, QString signal, QString &color1, QString &color2) {
    if (rawSignalsStatus.contains(signal + "|UU") && rawSignalsStatus[signal + "|UU"] == 0) {
        color1 = "Yellow";
        color2 = "Yellow";
    } else {
        if (rawSignalsStatus[signal + "|U"] == 0) {
            color1 = "Yellow";
        }
        if (rawSignalsStatus[signal + "|L"] == 0) {
            if (color1.isEmpty()) {
                color1 = "Green";
            } else {
                color2 = "Green";
            }
        }
        if (rawSignalsStatus[signal + "|H"] == 0) {
            if (color1.isEmpty()) {
                color1 = "Red";
            } else {
                color2 = "Red";
            }
        }
        if (rawSignalsStatus[signal + "|B"] == 0) {
            if (color1.isEmpty()) {
                color1 = "White";
            } else {
                color2 = "White";
            }
        }
    }
    if (color1.isEmpty() && color2.isEmpty()) {
        color1 = "Black";
        color2 = "Black";
    }
}


void TracktologyBuilder::updateRealSwithStatus(QByteArray ba){
    if (ba.length()!=2){
        return;
    }

    this->rawPhySwitchsStatus["4|DB"] = ba[1] & 0x01;       //表示4号道岔定标
    this->rawPhySwitchsStatus["4|FB"] = (ba[1] & 0x02)>>1;   //表示4号道岔反标
    this->rawPhySwitchsStatus["4|DBJ"] = (ba[1] & 0x04)>>2;  //表示4号道岔定标
    this->rawPhySwitchsStatus["4|FBJ"] = (ba[1] & 0x08)>>3;  //表示4号道岔反标
    qDebug()<<"4 switch status updated....DB FB DBJ FBJ"<<this->rawPhySwitchsStatus["4|DB"]
        <<this->rawPhySwitchsStatus["4|FB"]<<this->rawPhySwitchsStatus["4|DBJ"]<<this->rawPhySwitchsStatus["4|FBJ"];
    if(this->rawPhySwitchsStatus["4|DB"]==0x00 &&
            this->rawPhySwitchsStatus["4|FB"]==0x01 &&
            this->rawPhySwitchsStatus["4|DBJ"]==0x00 &&
            this->rawPhySwitchsStatus["4|FBJ"]==0x01){
        //'Lost', 'Normal', 'Reverse'
        da.updateTurnoutStatus("4", "Normal");
        emit turoutStatusUpdated("4", "Normal");
        return;
    }
    if(this->rawPhySwitchsStatus["4|DB"]==0x01 &&
            this->rawPhySwitchsStatus["4|FB"]==0x00 &&
            this->rawPhySwitchsStatus["4|DBJ"]==0x01 &&
            this->rawPhySwitchsStatus["4|FBJ"]==0x00){
        qDebug() << "应该运行到这里，更新啊！！！!!!!......MBBBBB....";
        da.updateTurnoutStatus("4", "Reverse");
        emit turoutStatusUpdated("4", "Reverse");
        return;
    }

    da.updateTurnoutStatus("4", "Lost");
    emit turoutStatusUpdated("4", "Lost");
    return;
    //如果不是这两种情况，则
}

//将其更新到数据库中
void TracktologyBuilder::updatTrackSectionStatus(QByteArray ba,int classno){
    if(ba.length()!=2){
            return;
     }

    if (classno==1){

            rawSegmentsStatus["3-11DG|FQJH"]= ba[1] & 0x01;
            rawSegmentsStatus["3-11DG|FQJQ"]= (ba[1] & 0x02)>>1;
            rawSegmentsStatus["3-11DG|GJQ"]= (ba[1] & 0x04)>>2;
            rawSegmentsStatus["IG|FQJH"]= (ba[1] & 0x08)>>3;
            rawSegmentsStatus["IG|FQJQ"]= (ba[1] & 0x10)>>4;
            rawSegmentsStatus["IG|GJQ"]= (ba[1] & 0x20)>>5;
            rawSegmentsStatus["4DG|FQJH"]= (ba[1] & 0x40)>>6;
            rawSegmentsStatus["4DG|FQJQ"]= (ba[1] & 0x80)>>7;
            rawSegmentsStatus["4DG|GJQ"]= ba[0] & 0x01;
            rawSegmentsStatus["3G|FQJH"]= (ba[0] & 0x02)>>1;
            rawSegmentsStatus["3G|FQJQ"]= (ba[0] & 0x04)>>2;
            rawSegmentsStatus["3G|GJQ"]= (ba[0] & 0x08)>>3;

            qDebug()<<"....updatekeepSegmentstatus....3-11G 1G 4DG 3G"<<
                rawSegmentsStatus["3-11DG|FQJH"]<<
                rawSegmentsStatus["3-11DG|FQJQ"]<<
                rawSegmentsStatus["3-11DG|GJQ"]<<
                rawSegmentsStatus["IG|FQJH"]<<
                rawSegmentsStatus["IG|FQJQ"]<<
                rawSegmentsStatus["IG|GJQ"]<<
                rawSegmentsStatus["4DG|FQJH"]<<
                rawSegmentsStatus["4DG|FQJQ"]<<
                rawSegmentsStatus["4DG|GJQ"]<<
                rawSegmentsStatus["3G|FQJH"]<<
                rawSegmentsStatus["3G|FQJQ"]<<
                rawSegmentsStatus["3G|GJQ"]
            ;

            QStringList mListstus;
            auto tmpst = this->InspectSegmentStatusFromRawData("3-11DG",rawSegmentsStatus);
            da.updateTrackSectionStatus("3-11DG",tmpst);
            mListstus.append(tmpst);
            auto tmpst1 = this->InspectSegmentStatusFromRawData("IG",rawSegmentsStatus);
            da.updateTrackSectionStatus("IG",tmpst1);
            mListstus.append(tmpst1);
            auto tmpst2 = this->InspectSegmentStatusFromRawData("4DG",rawSegmentsStatus);
            da.updateTrackSectionStatus("4DG",tmpst2);
            mListstus.append(tmpst2);
            auto tmpst3 = this->InspectSegmentStatusFromRawData("3G",rawSegmentsStatus);
            da.updateTrackSectionStatus("3G",tmpst3);
            mListstus.append(tmpst3);

            emit trackSectionUpdated({ "3-11DG","IG", "4DG" ,"3G" }, mListstus);
        


        }else if (classno==2){
                rawSegmentsStatus["X1LQG|GFJH"]= ba[1] & 0x01;
                rawSegmentsStatus["X1LQG|GFJQ"]= (ba[1] & 0x02)>>1;
                rawSegmentsStatus["X1LQG|GJQ"]= (ba[1] & 0x04)>>2;
                rawSegmentsStatus["1101G|GFJH"]= (ba[1] & 0x08)>>3;
                rawSegmentsStatus["1101G|GFJQ"]= (ba[1] & 0x10)>>4;
                rawSegmentsStatus["1101G|GJQ"]= (ba[1] & 0x20)>>5;
                rawSegmentsStatus["1103G|GFJH"]= (ba[1] & 0x40)>>6;
                rawSegmentsStatus["1103G|GFJQ"]= (ba[1] & 0x80)>>7;
                rawSegmentsStatus["1103G|GJQ"]= ba[0] & 0x01;
                rawSegmentsStatus["1105G|GFJH"]= (ba[0] & 0x02)>>1;
                rawSegmentsStatus["1105G|GFJQ"]= (ba[0] & 0x04)>>2;
                rawSegmentsStatus["1105G|GJQ"]=  (ba[0] & 0x08)>>3;
                rawSegmentsStatus["1107G|GFJH"]= (ba[0] & 0x10)>>4;
                rawSegmentsStatus["1107G|GFJQ"]= (ba[0] & 0x20)>>5;
                rawSegmentsStatus["1107G|GJQ"]=  (ba[0] & 0x40)>>6;

                QStringList tracksectionstatats;
                auto tmpst = this->InspectSegmentStatusFromRawData("X1LQG",rawSegmentsStatus);
                da.updateTrackSectionStatus("X1LQG",tmpst);
                tracksectionstatats.append(tmpst);
                auto tmpst1 = this->InspectSegmentStatusFromRawData("1101G",rawSegmentsStatus);
                da.updateTrackSectionStatus("1101G",tmpst1);
                tracksectionstatats.append(tmpst1);
                auto tmpst3 = this->InspectSegmentStatusFromRawData("1103G",rawSegmentsStatus);
                da.updateTrackSectionStatus("1103G",tmpst3);
                tracksectionstatats.append(tmpst3);
                auto tmpst4 = this->InspectSegmentStatusFromRawData("1105G",rawSegmentsStatus);
                da.updateTrackSectionStatus("1105G",tmpst4);
                tracksectionstatats.append(tmpst4);
                auto tmpst5 = this->InspectSegmentStatusFromRawData("1107G",rawSegmentsStatus);
                da.updateTrackSectionStatus("1107G",tmpst5);
                tracksectionstatats.append(tmpst5);
                emit trackSectionUpdated({ "X1LQG","1101G", "1103G" ,"1105G","1107G"}, tracksectionstatats);


    }else if(classno==10){
               rawSegmentsStatus["1-9DG|FQJH"]= ba[1] & 0x01;
               rawSegmentsStatus["1-9DG|FQJQ"]= (ba[1] & 0x02)>>1;
               rawSegmentsStatus["1-9DG|GJQ"]= (ba[1] & 0x04)>>2;
               rawSegmentsStatus["IIG|FQJH"]= (ba[1] & 0x08)>>3;   //1, 0改变方向了， 01没有改变方向
               rawSegmentsStatus["IIG|FQJQ"]= (ba[1] & 0x10)>>4;
               rawSegmentsStatus["IIG|GJQ"]= (ba[1] & 0x20)>>5;
               rawSegmentsStatus["2DG|FQJH"]= (ba[1] & 0x40)>>6;
               rawSegmentsStatus["2DG|FQJQ"]= (ba[1] & 0x80)>>7;
               rawSegmentsStatus["2DG|GJQ"]= ba[0] & 0x01;
               rawSegmentsStatus["4G|FQJH"]= (ba[0] & 0x02)>>1;
               rawSegmentsStatus["4G|FQJQ"]= (ba[0] & 0x04)>>2;
               rawSegmentsStatus["4G|GJQ"]=  (ba[0] & 0x08)>>3;

               QStringList tracksectionstatats;
               auto tmpst = this->InspectSegmentStatusFromRawData("1-9DG",rawSegmentsStatus);
               da.updateTrackSectionStatus("1-9DG",tmpst);
               tracksectionstatats.append(tmpst);
               auto tmpst1 = this->InspectSegmentStatusFromRawData("IIG",rawSegmentsStatus);
               da.updateTrackSectionStatus("IIG",tmpst1);
               tracksectionstatats.append(tmpst1);
               auto tmpst2 = this->InspectSegmentStatusFromRawData("2DG",rawSegmentsStatus);
               da.updateTrackSectionStatus("2DG",tmpst2);
               tracksectionstatats.append(tmpst2);
               auto tmpst3 = this->InspectSegmentStatusFromRawData("4G",rawSegmentsStatus);
               da.updateTrackSectionStatus("4G",tmpst3);
               tracksectionstatats.append(tmpst3);
               emit trackSectionUpdated({ "1-9DG","IIG", "2DG" ,"4G" }, tracksectionstatats);


    }

    else if(classno==11){
            rawSegmentsStatus["S1LQG|GFJH"]= ba[1] & 0x01;
            rawSegmentsStatus["S1LQG|GFJQ"]= (ba[1] & 0x02)>>1;
            rawSegmentsStatus["S1LQG|GJQ"]= (ba[1] & 0x04)>>2;
            rawSegmentsStatus["1102G|GFJH"]= (ba[1] & 0x08)>>3;
            rawSegmentsStatus["1102G|GFJQ"]= (ba[1] & 0x10)>>4;
            rawSegmentsStatus["1102G|GJQ"]= (ba[1] & 0x20)>>5;
            rawSegmentsStatus["1104G|GFJH"]= (ba[1] & 0x40)>>6;
            rawSegmentsStatus["1104G|GFJQ"]= (ba[1] & 0x80)>>7;
            rawSegmentsStatus["1104G|GJQ"]= ba[0] & 0x01;
            rawSegmentsStatus["1106G|GFJH"]= (ba[0] & 0x02)>>1;
            rawSegmentsStatus["1106G|GFJQ"]= (ba[0] & 0x04)>>2;
            rawSegmentsStatus["1106G|GJQ"]=  (ba[0] & 0x08)>>3;
            rawSegmentsStatus["1108G|GFJH"]= (ba[0] & 0x10)>>4;
            rawSegmentsStatus["1108G|GFJQ"]= (ba[0] & 0x20)>>5;
            rawSegmentsStatus["1108G|GJQ"]=  (ba[0] & 0x40)>>6;
            QStringList tracksectionstatats;
            auto tmpst = this->InspectSegmentStatusFromRawData("S1LQG",rawSegmentsStatus);
            da.updateTrackSectionStatus("S1LQG",tmpst);
            tracksectionstatats.append(tmpst);

            auto tmpst1 = this->InspectSegmentStatusFromRawData("1102G",rawSegmentsStatus);
            da.updateTrackSectionStatus("1102G",tmpst1);
            tracksectionstatats.append(tmpst1);

            auto tmpst2 = this->InspectSegmentStatusFromRawData("1104G",rawSegmentsStatus);
            da.updateTrackSectionStatus("1104G",tmpst2);
            tracksectionstatats.append(tmpst2);

            auto tmpst3 = this->InspectSegmentStatusFromRawData("1106G",rawSegmentsStatus);
            da.updateTrackSectionStatus("1106G",tmpst3);
            tracksectionstatats.append(tmpst3);

            auto tmpst4 = this->InspectSegmentStatusFromRawData("1108G",rawSegmentsStatus);
            da.updateTrackSectionStatus("1108G",tmpst4);
            tracksectionstatats.append(tmpst4);

            emit trackSectionUpdated({ "S1LQG","1102G", "1104G" ,"1106G","1108G"}, tracksectionstatats);

            

        }else{
        return;
    }

}

//    "status TEXT CHECK(status IN ('Unknown', 'Free', 'Occupied'))"

QString TracktologyBuilder::InspectSegmentStatusFromRawData(QString segment_name,
                                             QMap<QString, unsigned char>&
                                        segmentsStatus) {
    if(segmentsStatus.size()<=0){
           return "Unknown";
       }
       if(segmentsStatus.size()>0){
           auto the_segmentkeys=segmentsStatus.keys();
           bool existed = false;
           for (int i = 0; i < the_segmentkeys.size(); i++) {
               auto tmpkey = the_segmentkeys.at(i).split("|").first();
               if(tmpkey==segment_name){
                   existed = true;
                   break;
               }
           }
           if(existed==false){
               return "Unknown";
           }
       }
       QStringList segmentIds = {"3-11DG", "IG", "4DG","3G","1-9DG","IIG","2DG","4G"};

       for (const QString& id: segmentIds) {
             if(id!=segment_name){
                 continue;
             }
             QString status = inspectSegmentStatusFromRawData(id, 0x00, 0x01, 0x01);
             if (status != "Unknown") {
                 return status;
             }
        }


       QStringList segmentId1 = {"X1LQG", "1101G", "1103G","1105G","1107G","1102G","1104G","1106G","1108G","S1LQG"};

       for (const QString& id: segmentId1) {
             if(id!=segment_name){
                 continue;
             }
             QString status = inspectSegmentStatusFromRawData(id);
             if (status != "Unknown") {
                 return status;
             }
        }

       return "Unknown";

}

