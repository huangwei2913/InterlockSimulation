#ifndef SEGMENTCIRCUITDIAGRAM_H
#define SEGMENTCIRCUITDIAGRAM_H

#include <QWidget>
#include <QPushButton>
#include "CppObject.h"
#include <QQuickView>

namespace Ui {
class SegmentCircuitDiagram
        ;
}

//在这里现实另外窗口
class SegmentCircuitDiagram
        : public QWidget
{
    Q_OBJECT

public:
    explicit SegmentCircuitDiagram
    (QWidget *parent = nullptr);
    ~SegmentCircuitDiagram
    ();

    uint16_t ModbusCRC16(QByteArray senddata)
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

    //判断是否为数字
    int isDigitStr(QString src)
    {
        QByteArray ba = src.toLatin1();//QString 转换为 char*
        const char *s = ba.data();

        while(*s && *s>='0' && *s<='9') s++;

        if (*s)
        { //不是纯数字
            return -1;
        }
        else
        { //纯数字
            return 0;
        }
    }


public slots:

    void emitbacktomainsignal();
    void sendMessageToxiaweiji();       //想下位机发送消息
    void setNextGuzhangBan();          //设置下一块故障版
    void resetAllGuzhang();            //复位所有故障
    void signalFaultSet();              //设置单点故障

signals:
    void backtomain();
    void sendMessageToseriesport(QByteArray ba);

private:
    Ui::SegmentCircuitDiagram*ui;
    QPushButton* btn_room_setting;
    CppObject* cppObj;
    QWidget *container;
    QQuickView * myview;
    QStringList qmlfileList;
    double the_next_clicked;     //点设置下一个故障，可以点到黄昏去


};

#endif // SEGMENTCIRCUITDIAGRAM_H
