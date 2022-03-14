#include "SeriesPortTask.h"
#include<QTime>
#include <QDebug>


SeriesPortTask::SeriesPortTask(QByteArray ba, ReceiverThread* seriesportthread)
{
  this->ba = ba;
  this->portthread = seriesportthread;
}


SeriesPortTask::~SeriesPortTask(){
    qDebug()<<"finished the tasks....."<<ba.toHex();
}

void SeriesPortTask::run(){
    if(ba.size()>0){
        unsigned char temp =(unsigned char)ba[0];
        if(0xc0<=temp && temp<=0xc9){
            portthread->sendMessageToSeriealPort(ba);
             QThread::msleep(1000);
        }
        if(0xfa==temp){
            portthread->sendMessageToSeriealPort(ba);
             QThread::msleep(200);
        }
    }
}

