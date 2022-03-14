#ifndef HELPER_H
#define HELPER_H

#include <QByteArray>
#include <QApplication>
#include <QEvent>
#include <QTime>
#include <QTimer>



class Helper
{
public:
    Helper();
    static  uint16_t ModbusCRC16(QByteArray senddata)
    {
        int len=senddata.size();
        uint16_t wcrc=0XFFFF;//预置16位crc寄存器，初值全部为1
        uint8_t temp;//定义中间变量
        int i=0,j=0;//定义计数
        for(i=0;i<len;i++)//循环计算每个数据
        {
           temp=senddata.at(i);
           wcrc^=temp;
           for(j=0;j<8;j++){
              //判断右移出的是不是1，如果是1则与多项式进行异或。
              if(wcrc&0X0001){
                  wcrc>>=1;//先将数据右移一位
                  wcrc^=0XA001;//与上面的多项式进行异或
              }
              else//如果不是1，则直接移出
                  wcrc>>=1;//直接移出
           }
        }
        temp=wcrc;//crc的值
        return wcrc;
    }

    static void DelaySpecifyTimePeriod(int millseconds){
        QTime t = QTime::currentTime();
        QTime s;
        s = t.addMSecs(millseconds);
        while(s!=QTime::currentTime()){
           QCoreApplication::processEvents();
        }
    }

    const static unsigned char color2byte_1102_L = 0xfe;
    const static  unsigned char color2byte_1102_H = 0xfd;
    const  static  unsigned char color2byte_1102_U = 0xfb;
    const  static   unsigned char color2byte_1104_L = 0xf7;
    const static  unsigned char color2byte_1104_H = 0xef;
    const  static  unsigned char color2byte_1104_U = 0xdf;
    const  static  unsigned char color2byte_1106_L = 0xfe;
    const  static  unsigned char color2byte_1106_H = 0xdf;
    const  static  unsigned char color2byte_1106_U = 0xfb;
    const  static unsigned char color2byte_1108_L = 0xf7;
    const   static unsigned char color2byte_1108_H = 0xef;
    const  static unsigned char color2byte_1108_U = 0xdf;
    const static unsigned char color2byte_S4_L = 0xfe;
    const static unsigned char color2byte_S4_H = 0xfd;
    const static unsigned char color2byte_S4_U = 0xfb;
    const static unsigned char color2byte_S4_FB = 0xf7;
    const static unsigned char color2byte_XII_L = 0xef;
    const static unsigned char color2byte_XII_H = 0xdf;
    const static unsigned char color2byte_SII_L = 0xfe;
    const static unsigned char color2byte_SII_H = 0xfd;
    const static unsigned char color2byte_SII_U = 0xfb;
    const static unsigned char color2byte_SII_FB = 0xf7;
    const static unsigned char color2byte_S3_L = 0xef;
    const static unsigned char color2byte_S3_H = 0xdf;
    const static unsigned char color2byte_S3_U = 0xbf;
    const static unsigned char color2byte_S3_FB = 0x7f;

    const static unsigned char color2byte_XN_2U = 0xfe;
    const static unsigned char color2byte_XN_YB = 0xfd;
    const static unsigned char color2byte_S1_L = 0xfb;
    const static unsigned char color2byte_S1_H = 0xf7;
    const static unsigned char color2byte_S1_U = 0xef;
    const static unsigned char color2byte_S1_FB = 0xdf;

    const static unsigned char color2byte_X_U = 0xfe;
    const static unsigned char color2byte_X_L = 0xfd;
    const static unsigned char color2byte_X_H = 0xfb;
    const static unsigned char color2byte_X_2U = 0xf7;
    const static unsigned char color2byte_X_YB = 0xef;
    const static unsigned char color2byte_XN_U = 0xdf;
    const static unsigned char color2byte_XN_L = 0xbf;
    const static unsigned char color2byte_XN_H = 0x7f;



    const static  unsigned char color2byte_1107_L = 0xfe;
    const static  unsigned char color2byte_1107_H = 0xfd;
    const static  unsigned char color2byte_1107_U = 0xfb;
    const static  unsigned char color2byte_1103_L = 0xfe;
    const static  unsigned char color2byte_1103_H = 0xfd;
    const static  unsigned char color2byte_1103_U = 0xfb;
    const static  unsigned char color2byte_1105_L = 0xf7;
    const static  unsigned char color2byte_1105_H = 0xef;
    const static  unsigned char color2byte_1105_U = 0xdf;


    const static unsigned char color2byte_XI_H = 0xf7;
    const static unsigned char color2byte_XI_U = 0xfd;
    const static unsigned char color2byte_XI_L = 0xf9;
    const static unsigned char color2byte_XI_LU = 0xf5;
    const static unsigned char color2byte_1101_U = 0xbf;
    const static unsigned char color2byte_1101_H = 0xdf;
    const static unsigned char color2byte_1101_L = 0xef;
    const static unsigned char color2byte_1101_LU = 0x7f;

    const static unsigned char color2byte_SN_2U = 0xfe;
    const static unsigned char color2byte_SN_YB = 0xfd;
    const static  unsigned char color2byte_X3_L = 0xfb;
    const static unsigned char color2byte_X3_H = 0xf7;
    const static unsigned char color2byte_X3_U = 0xef;
    const static unsigned char color2byte_X4_L = 0xdf;
    const static unsigned char color2byte_X4_H = 0xbf;
    const static  unsigned char color2byte_S_U = 0xfe;
    const static   unsigned char color2byte_S_L = 0xfd;
    const static  unsigned char color2byte_S_H = 0xfb;
    const static  unsigned char color2byte_S_2U = 0xf7;
    const static  unsigned char color2byte_S_YB = 0xef;
    const static   unsigned char color2byte_SN_U = 0xdf;
    const static    unsigned char color2byte_SN_L = 0xbf;
    const static  unsigned char color2byte_SN_H = 0x7f;

    //all 1,3,5,7,11 switch DC


};

#endif // HELPER_H
