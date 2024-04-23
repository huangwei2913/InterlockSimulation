#pragma once

#include <QObject>
#include <QtMath>  // 添加这一行
#include "Configurations.h"
#include <QVector>
#include <QMap>
//一些小的trick类和方法都存储在这里
class Utilty  : public QObject
{
	Q_OBJECT

public:
	Utilty(QObject *parent);
	~Utilty();


    static QVector<int> generateAheadCircleNumber(int start, int size) {
        QVector<int> zz;
        for (int j = 1; j <= size - 1; j++) {
            int tmp = start + j;
            if (tmp < size) {
                zz.append(tmp);
            }
            else {
                zz.append(tmp % size);
            }
        }
        return zz;
    }

    
    static QString removeLastSpiltCharacter(QString data) {

        auto preconditon = data.back();
        if (preconditon == '|') {
            auto the_loc = data.lastIndexOf("|");
            QString finasignalNeedUpdates = data;
            if (the_loc != -1) {
                finasignalNeedUpdates = data.mid(0, the_loc);
            }
            return finasignalNeedUpdates;
        }
        else {
            return data;
        }
    }

	static QStringList obtainSignalsFromInroutePath( const QString & inroutepath ) {
		QStringList pathElements = inroutepath.split(" -> ");
		QStringList result;

		for (const QString& element : pathElements)
		{
			auto t_element = element.trimmed();
			bool isNumber;
			t_element.toInt(&isNumber);
			if (!isNumber) {
				result.append(t_element);
			}
		}
		return result;
	}

	static QStringList obtainSwitchNumbersFromInroutePath(const QString& inroutepath) {
		QStringList pathElements = inroutepath.split(" -> ");
		QStringList result;

		for (const QString& element : pathElements)
		{
			auto t_element = element.trimmed();
			bool isNumber;
			t_element.toInt(&isNumber);
			if (isNumber) {
				result.append(t_element);
			}
		}
		return result;
	}



    static  uint16_t ModbusCRC16(QByteArray senddata)
    {
        int len = senddata.size();
        uint16_t wcrc = 0XFFFF;//预置16位crc寄存器，初值全部为1
        uint8_t temp;//定义中间变量
        int i = 0, j = 0;//定义计数
        for (i = 0; i < len; i++)//循环计算每个数据
        {
            temp = senddata.at(i);
            wcrc ^= temp;
            for (j = 0; j < 8; j++) {
                //判断右移出的是不是1，如果是1则与多项式进行异或。
                if (wcrc & 0X0001) {
                    wcrc >>= 1;//先将数据右移一位
                    wcrc ^= 0XA001;//与上面的多项式进行异或
                }
                else//如果不是1，则直接移出
                    wcrc >>= 1;//直接移出
            }
        }
        temp = wcrc;//crc的值
        return wcrc;
    }


    static QByteArray appendCRC(const QByteArray& ba) {
        QByteArray result = ba;
        result.resize(ba.size() + 2);
        uint16_t wcrc1 = ModbusCRC16(ba);
        unsigned char di1 = static_cast<unsigned char>(wcrc1);
        unsigned char gao1 = static_cast<unsigned char>(wcrc1 >> 8);
        result[result.size() - 2] = di1;
        result[result.size() - 1] = gao1;
        return result;
    }

    static unsigned char convertStrToByte(QString str) {
        int tempain = 0;
        int j = 0;
        for (int i = 7; i >= 0; i--) {
            tempain = tempain + (str.at(j) == '1' ? 1 * qPow(2, i) : 0);
            j = j + 1;
        }
        return  (unsigned char)tempain;
    }

    //得到所有前向信号机
  


    //只适用于寻找站内区段名称，用坐标起点和终点
 static QString getInstationTrackSectionNameWithStartOrEndSignals(QString startSignal, QString endSignal) {
  
        if (Configurations::instationSignals.contains(startSignal) && Configurations::instationSignals.contains(endSignal)) {
            if (startSignal == "X" && endSignal == "S3") {
                return "3-11DG";
            }
            if (startSignal == "X" && endSignal == "SI") {
                return "3-11DG";
            }
   
            if (startSignal == "SN" && endSignal == "X3") {
                return "4DG";
            }
 
            if (startSignal == "SN" && endSignal == "XI") {
                return "4DG";
            }

            if (startSignal == "X4" && endSignal == "S") {
                return "2DG";
            }
            if (startSignal == "XII" && endSignal == "S") {
                return "2DG";
            }

            if (startSignal == "XN" && endSignal == "SII") {
                return "1-9DG";
            }
            if (startSignal == "XN" && endSignal == "S4") {
                return "1-9DG";
            }

            if (startSignal == "S3" && endSignal == "X3") {
                return "3G";
            }
            if (startSignal == "SI" && endSignal == "XI") {
                return "IG";
            }
            if (startSignal == "SII" && endSignal == "XII") {
                return "IIG";
            }
            if (startSignal == "S4" && endSignal == "X4") {
                return "4G";
            }
            return "";
        }
        return "";
  }

 static QString calculateFaMaAdvancedOneStep(QString the_ma) {
     if (the_ma == "U") {
         return "LU";
     }
     if (the_ma == "LU") {
         return "L";
     }
     if (the_ma == "L") {
         return "L2";
     }
     if (the_ma == "L2") {
         return "L3";
     }
     if (the_ma == "L3") {
         return "L4";
     }
     if (the_ma == "HU") {
         return "U";
     }
 }

 static QVector<QString> rectifiedFaMaInfo(QMap<QString, QString> finalFama) {
     QVector<QString> result;
     QString segsss = "";
     QString the_maxx = "";
     int i = 0;
     for (QString seg : finalFama.keys()) {
         segsss += finalFama.keys().at(i);
         segsss += "|";
         the_maxx += finalFama.value(finalFama.keys().at(i));
         the_maxx += "|";
         i++;
     }
     result.append(segsss);
     result.append(the_maxx);
     return result;
 }

 //这个函数只适合跨线运行时候的信号机序列生成
 static QString getTraversingSignalsByFromAndendWithInroutePath(QString routepath) {
     auto thestart = routepath.split(" -> ").first().trimmed();
     auto theend = routepath.split(" -> ").last().trimmed();
     auto allsgs = obtainSignalsFromInroutePath(routepath);
     if (thestart == "X" && (allsgs.contains("S4") || allsgs.contains("X4"))) {
         return "X|S4|X4|S|1102|1104|1106|1108";
     }
     if (thestart == "X" && (allsgs.contains("SII") || allsgs.contains("XII"))) {
         return "X|SII|XII|S|1102|1104|1106|1108";
     }
     if (thestart == "XN" && (allsgs.contains("S3")|| allsgs.contains("X3"))) {
         return "XN|S3|X3|SN|1101|1103|1105|1107";
     }
     if (thestart == "XN" && (allsgs.contains("SI") || allsgs.contains("XI"))) {
         return "XN|SI|XI|SN|1101|1103|1105|1107";
     }

     if (theend == "X" && (allsgs.contains("XII") || allsgs.contains("SII"))) {
         return "1108|1106|1104|1102|S|XII|SII|X";
     }

     if (theend == "X" && (allsgs.contains("X4") || allsgs.contains("S4"))) {
         return "1108|1106|1104|1102|S|X4|S4|X";
     }

     if (theend == "XN" && (allsgs.contains("X3") || allsgs.contains("S3"))) {
         return "1107|1105|1103|1101|SN|X3|S3|XN";
     }

     if (theend == "XN" && (allsgs.contains("SI") || allsgs.contains("XI"))) {
         return "1107|1105|1103|1101|SN|XI|SI|XN";
     }
     return "";
 }

 //这个函数只适合跨线运行时候的区段序列生成
 static QString getTraversingSegmentsByFromAndendWithInroutePath(QString routepath) {
     auto thestart = routepath.split(" -> ").first().trimmed();
     auto theend = routepath.split(" -> ").last().trimmed();
     auto allsgs = obtainSignalsFromInroutePath(routepath);
     if (thestart == "X" && (allsgs.contains("S4") || allsgs.contains("X4"))) {
         return "1107G|4G|2DG|1102|1104|1106|1108|S1LQG";
     }
     if (thestart == "X" && (allsgs.contains("SII") || allsgs.contains("XII"))) {
         return "1107G|IIG|2DG|1102|1104|1106|1108|S1LQG";
     }
     if (thestart == "XN" && (allsgs.contains("S3") || allsgs.contains("X3"))) {
         return "S1LQG|3G|4DG|X1LQG|1101G|1103G|1105G|1107G";
     }
     if (thestart == "XN" && (allsgs.contains("SI") || allsgs.contains("XI"))) {
         return "S1LQG|IG|4DG|X1LQG|1101G|1103G|1105G|1107G";
     }

     if (theend == "X" && (allsgs.contains("XII") || allsgs.contains("SII"))) {
         return "S1LQG|1108G|1106G|1104G|1102G|2DG|IIG|1107G";
     }

     if (theend == "X" && (allsgs.contains("X4") || allsgs.contains("S4"))) {
         return "S1LQG|1108G|1106G|1104G|1102G|2DG|4G|1107G";
     }

     if (theend == "XN" && (allsgs.contains("X3") || allsgs.contains("S3"))) {
         return "1107G|1105G|1103G|1101G|XlLQG|4DG|3G|S1LQG";
     }

     if (theend == "XN" && (allsgs.contains("SI") || allsgs.contains("XI"))) {
         return "1107G|1105G|1103G|1101G|XlLQG|4DG|IG|S1LQG";
     }
     return "";
 }
 //这个函数只适合跨线运行时候的补充区段序列生成
 static QString getTraversingCompletorySegmentsByFromAndendWithInroutePath(QString routepath) {
     auto thestart = routepath.split(" -> ").first().trimmed();
     auto theend = routepath.split(" -> ").last().trimmed();
     auto allsgs = obtainSignalsFromInroutePath(routepath);
     if (thestart == "X" && (allsgs.contains("S4") || allsgs.contains("X4"))) {
         return "IIG";
     }
     if (thestart == "X" && (allsgs.contains("SII") || allsgs.contains("XII"))) {
         return "4G";
     }
     if (thestart == "XN" && (allsgs.contains("S3") || allsgs.contains("X3"))) {
         return "IG";
     }
     if (thestart == "XN" && (allsgs.contains("SI") || allsgs.contains("XI"))) {
         return "3G";
     }

     if (theend == "X" && (allsgs.contains("XII") || allsgs.contains("SII"))) {
         return "4G";
     }

     if (theend == "X" && (allsgs.contains("X4") || allsgs.contains("S4"))) {
         return "IIG";
     }

     if (theend == "XN" && (allsgs.contains("X3") || allsgs.contains("S3"))) {
         return "IG";
     }

     if (theend == "XN" && (allsgs.contains("SI") || allsgs.contains("XI"))) {
         return "3G";
     }
     return "";
 }


 //跨线路运行的时候，到底应该在那条线路上发码
 static QString WhatLineShouldFaMa(QString routepath) {
     auto thestart = routepath.split(" -> ").first().trimmed();
     auto theend = routepath.split(" -> ").last().trimmed();
     if (thestart == "X") {
         return "Up";
     }
     if (thestart == "XN") {
         return "Down";
     }

     if (theend == "XN") {
         return "Down";
     }

     if (theend == "X") {
         return "Up";
     }

 }

 static QString WhatLineTheSectionIsIn(QString section) {
    
     if (Configurations::downwardTrackSections.contains(section) || section == "3G") {
         return "Down";
     }
    
     if (Configurations::upwardTrackSections.contains(section) || section == "4G") {
         return "Up";
     }
     return "";
 }



 static QString intToQString(unsigned int value) {
     // 将int类型转换为unsigned short
     unsigned short ushortValue = static_cast<unsigned short>(value);

     // 将unsigned short类型转换为QString，并以二进制形式存储
     QString binaryString = QString::number(ushortValue, 2);

     // 如果binaryString的长度小于16，那么在前面补0
     while (binaryString.length() < 16) {
         binaryString.prepend('0');
     }

     return binaryString;
 }

 
 static unsigned short bytesToUShort(unsigned char byte1, unsigned char byte2) {
     // 将两个字节合并为一个无符号短整型
     unsigned short ushortValue = (byte1 << 8) | byte2;

     return ushortValue;
 }

 //只有四种的情况
 static QString getSignalColorFrom4Combination(unsigned char L, unsigned char H, unsigned char U, unsigned char B) {
     QString signalStatus = QString::number(L) + QString::number(H) + QString::number(U) + QString::number(B);

     QString color1="Black", color2="Black";
     if (signalStatus.count('0') >= 3 || signalStatus == "0011" || signalStatus == "1001") {
         color1 = color2 = "Black";
     }
     else if (signalStatus.count('0') == 2) {
         if (signalStatus == "1100") {
             color1 = "Yellow";
             color2 = "White";
         }
         else if (signalStatus == "1010") {
             color1 = "Red";
             color2 = "White";
         }
         else if (signalStatus == "0110") {
             color1 = "Green";
             color2 = "White";
         }
         else if (signalStatus == "0101") {
             color1 = "Green";
             color2 = "Yellow";
         }
     }
     else if (signalStatus.count('0') == 1) {
         if (L == 0) {
             color1 = "Green";
             color2 = "Black";
         }
         else if (H == 0) {
             color1 = "Red";
             color2 = "Black";
         }
         else if (U == 0) {
             color1 = "Yellow";
             color2 = "Black";
         }
         else if (B == 0) {
             color1 = "White";
             color2 = "Black";
         }
     }

     return color1 + "|" + color2;
 }


 static bool isAllDigits(const QString& str)
 {
     for (QChar c : str) {
         if (!c.isDigit()) {
             return false;
         }
     }
     return true;
 }


 //五种颜色组合
 static QString getSignalColorFrom5Combination(unsigned char L, unsigned char H, unsigned char U, unsigned char B, unsigned char UU)
 {
     QString signalStatus = QString::number(L) + QString::number(H) + QString::number(U) + QString::number(B) + QString::number(UU);

     QString color1 = "Black";
     QString color2 =  "Black";

     // 如果含有3个及以上的0,或者包含UU(双黄)且含有2个0,则颜色都为黑色
     if (signalStatus.count('0') >= 3)
     {
         color1 = color2 =  "Black";
     }
     // 如果含有2个0,且不包含UU(双黄),则颜色为白黄
     else if (signalStatus.count('0') == 2 )
     {
         if (signalStatus == "01011") {
             color1 = "Green";
             color2 = "Yellow";
         }
         else if (signalStatus == "01101") {
             color1 = "Green";
             color2 = "White";
         }
         else if (signalStatus == "10101") {
             color1 = "Red";
             color2 = "White";
         }
         else if (signalStatus == "11001") {
             color1 = "Yellow";
             color2 = "White";
         }

     }
     // 如果含有1个0
     else if (signalStatus.count('0') == 1)
     {
         if (B == 0)
         {
             color1 = "White";
             color2 = "Black";
         }
         else if (UU == 0)
         {
             color1 = color2 = "Yellow";
         }
         else if (H == 0)
         {
             color1 = "Red";
             color2 = "Black";
         }
         else if (L == 0)
         {
             color1 = "Green";
             color2 = "Black";
         }
         else if (U == 0)
         {
             color1 = "Yellow";
             color2 = "Black";
         }
     }

     return color1 + "|" +color2;
 }



 static QString getSignalColorFrom3Combination(unsigned char L, unsigned char H, unsigned char U)
 {
     QString signalStatus = QString::number(L) + QString::number(H) + QString::number(U);

     QString color1 = "Black";
     QString color2 = "Black";

     // 如果含有3个0,则颜色都为黑色
     if (signalStatus.count('0') == 3)
     {
         color1 = color2 = "Black";
     }
     // 如果含有2个0
     else if (signalStatus.count('0') == 2)
     {
         if (signalStatus == "010" )
         {
             color1 = "Green";
             color2 = "Yellow";
         }
     }
     // 如果含有1个0
     else if (signalStatus.count('0') == 1)
     {
         if (L == 0)
         {
             color1 = "Green";
             color2 = "Black";
         }
         else if (H == 0)
         {
             color1 = "Red";
             color2 = "Black";
         }
         else if (U == 0)
         {
             color1 = "Yellow";
             color2 = "Black";
         }
     }

     return color1 + "|" + color2;
 }






};
