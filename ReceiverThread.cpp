#include "ReceiverThread.h"
#include <QSerialPort>
#include <QTime>
#include <QDebug>
#include "concurrentqueue.h"
extern moodycamel::ConcurrentQueue<std::string> commanditems;  //多个线程执行
void ReceiverThread::sendMessageToSeriealPort(QByteArray ba){
    serial.write(ba);
    serial.flush();
}

ReceiverThread::ReceiverThread(QObject *parent) :
    QThread(parent)
{
    m_response="hwhwhwhwh";    //作为测试用
    this->lastba_b8.resize(2); //作为测试用
    this->lastba_b9.resize(2); //作为测试用
    this->lastba_b2.resize(2);
    b8_update_flag=0;
    b9_update_flag=0;
    b2_update_flag=0;
}

ReceiverThread::~ReceiverThread()
{
    m_mutex.lock();
    m_quit = true;
    m_mutex.unlock();
    wait();
}

void ReceiverThread::startReceiver(const QString &portName, int waitTimeout)
{
    const QMutexLocker locker(&m_mutex);
    m_portName = portName;
    m_waitTimeout = waitTimeout;
    if (!isRunning())
        start();
}

void ReceiverThread::startSendMessage(QByteArray ba){
    serial.write(ba);
}

void ReceiverThread::currentPortChanged(const QString &text){

    m_mutex.lock();
    m_portName = text;
    m_mutex.unlock();
}
//轨道交通协议，协同培养任务设计最少带两个
QString  ReceiverThread::getSpecifiedLocationMessage(QString tobecheckedString, int locindex){

    return tobecheckedString.mid(locindex*10,10);
//    QString tempstr = tobecheckedString.replace(QString("29f5"), QString("|"));
//    auto firstchar = tempstr.front();
//    if(firstchar=='|'){
//        tempstr=tempstr.remove(0,1);
//    }
//    auto lastchar = tempstr.back();
//    if(lastchar=='|'){
//       tempstr = tempstr.remove(tempstr.length()-1,1);
//    }
//    auto strlist =tempstr.split("|");
//    return strlist[locindex];
}
int ReceiverThread::getCountofValidMessage(QString & tobecheckedString){
    if(tobecheckedString.length()<=0){
        return 0;
    }
    if(tobecheckedString.length()>0){

        if(tobecheckedString.contains("29f5")==true){

            if(tobecheckedString=="29f5"){ //judge if it is 0x29 0xf5
                return 0;
            }


            while(tobecheckedString.startsWith("29f5")==true){      //delete the head position 29f5
                tobecheckedString = tobecheckedString.mid(4);
            }

            while(tobecheckedString.endsWith("29f5")==true){      //delete the head position 29f5, should be tested
                tobecheckedString = tobecheckedString.mid(0,tobecheckedString.length()-4);
            }

            QString tempstr1 = tobecheckedString.replace(QString("29f5"), QString(""));  //29f5 in the middle of the string
            //应该是这个地方问题
            tempstr1=tempstr1.replace("|","");
            //qDebug()<<"tobecheckedString......tempstr1....."<<tempstr1<<"...."<<tempstr1.size();
            if(tempstr1==""){
                tobecheckedString = "";
                return 0;
            }
            if(tempstr1.length()/10>0){
                if(tempstr1.indexOf("b")!=0){   // I do not konow what this mean
                    tempstr1=tempstr1.mid(tempstr1.indexOf("b"),-1);
                }
                tobecheckedString = tempstr1;
                return tempstr1.length()/10;
            }
        }else{

            if(tobecheckedString.contains("29fd")==true){ // Is this the special case???

                if(tobecheckedString=="29fd"){ //judge if it is 0x29 0xfd
                    return 0;
                }



                while(tobecheckedString.startsWith("29fd")==true){      //delete the head position 29f5
                    tobecheckedString = tobecheckedString.mid(4);
                }

                while(tobecheckedString.endsWith("29fd")==true){      //delete the head position 29f5, should be tested
                    tobecheckedString = tobecheckedString.mid(0,tobecheckedString.length()-4);
                }


               QString tempstr1 = tobecheckedString.replace(QString("29fd"), QString("|"));
               QString tempstr2 = tobecheckedString.replace(QString("|"), QString(""));
               if(tempstr2.length()/10>0){
                   tobecheckedString = tempstr2;
                   return tempstr2.length()/10;
               }else{
                   return 0;
               }

            }else{
                //qDebug()<<"tobecheckedString的内容是..."<<tobecheckedString.length() <<"....."<<tobecheckedString.length()/10;
                if(tobecheckedString.length()>0){
                    return tobecheckedString.length()/10;
                }
                return 0;
            }

        }
    }

}

void ReceiverThread::run(){

    m_mutex.lock();

    if (currentPortName != m_portName) {
        currentPortName = m_portName;
        currentPortNameChanged = true;
    }
    int currentWaitTimeout = m_waitTimeout;
    QString currentRespone = m_response;
    m_mutex.unlock();

    while (!m_quit) {

        if (currentPortNameChanged) {
            serial.close();
            serial.setPortName(currentPortName);
            serial.setBaudRate(QSerialPort::Baud9600);
            serial.setDataBits(QSerialPort::Data8);
            serial.setStopBits(QSerialPort::OneStop);
            serial.setParity(QSerialPort::NoParity);
            if (!serial.open(QIODevice::ReadWrite)) {
                emit error(tr("Can't open %1, error code %2")
                           .arg(m_portName).arg(serial.error()));
                return;
            }

        }
        //十四五期间，培养的毕业生去到哪里，创新创业大赛组建，选聘授课，创新创业培训
        //初创项目--->创业团队--->产品的价值--->竞争对手分析--->商业模式--->投资回报-->风险分析
        QByteArray readData = serial.readAll();
        while (serial.waitForReadyRead(currentWaitTimeout))
            readData.append(serial.readAll());

        if (readData.length()>1){

            QString tobecheckedString = QString::fromUtf8(readData.toHex());
            //qDebug()<<"tobecheckedString......"<<tobecheckedString;
            int validMessagecount = getCountofValidMessage(tobecheckedString);
            for(int i=0;i<validMessagecount;i++){
                auto tempstriner = getSpecifiedLocationMessage(tobecheckedString,i);
                qDebug()<<"得到的有效消息是......"<<tempstriner<<"长度是"<<tempstriner.length();

                if(tempstriner.length()!=10){
                    continue;
                }
                QString message=tempstriner;
                QString finalinspectMessage="";   //因为这里还不能确认得到的消息长度就是10
                for(int j=0;j<5;j++){
                    finalinspectMessage.append(message.at(j*2));
                    finalinspectMessage.append(message.at(j*2+1));
                    finalinspectMessage.append('|');
                }
                auto pp = finalinspectMessage.remove(finalinspectMessage.length()-1,1);
               // qDebug()<<"the final string is the......."<<pp;
                auto temp_arrlist = pp.split('|');
                //auto temp_arrlist = request.split('|');
                //qDebug()<<" the first data is ...."<<temp_arrlist[0];
                //把temp_arrlist分离出来
                if(temp_arrlist[0]=="b1"){
                    if(temp_arrlist.length()==5){
                        bool ok;
                        QByteArray temp_data;
                        temp_data.resize(3);
                        for(int i=0;i<3;i++){
                            bool ok;
                            int hex =temp_arrlist[i].toInt(&ok,16); //hex=255;ok=true;
                            auto temba = temp_arrlist[i].toLocal8Bit();
                            temp_data[i] =QByteArray::fromHex(temba).front();
                        }
                        uint16_t wcrc = Helper::ModbusCRC16(temp_data);
                        unsigned char di=uint8_t(wcrc);
                        unsigned char gao=uint8_t(wcrc>>8);
                        int receiveddi = temp_arrlist[3].toInt(&ok,16);
                        int receivedgao = temp_arrlist[4].toInt(&ok,16);
                        int zzdi = (int)di;
                        int zzgao = (int)gao;
                        if(zzdi==receiveddi && zzgao==receivedgao){    //收到的数据完全正确，则更新那个
                            QByteArray returnba;
                            returnba.resize(2);
                            returnba[0] =temp_data[1];
                            returnba[1] =temp_data[2];
                            //qDebug()<<".........returnba........"<<returnba.toHex();
                            emit feishiwusignalstatusreceieved(returnba,9);
                        }
                    }else{
                        qDebug()<<".........enter concater secner ....";
                    }

                  }

                if(temp_arrlist[0]=="b2"){
                    if(temp_arrlist.length()==5){
                        bool ok;
                        QByteArray temp_data;
                        temp_data.resize(3);
                        for(int i=0;i<3;i++){
                            bool ok;
                            int hex =temp_arrlist[i].toInt(&ok,16); //hex=255;ok=true;
                            auto temba = temp_arrlist[i].toLocal8Bit();
                            temp_data[i] =QByteArray::fromHex(temba).front();
                        }
                        uint16_t wcrc = Helper::ModbusCRC16(temp_data);
                        unsigned char di=uint8_t(wcrc);
                        unsigned char gao=uint8_t(wcrc>>8);
                        int receiveddi = temp_arrlist[3].toInt(&ok,16);
                        int receivedgao = temp_arrlist[4].toInt(&ok,16);

                        int zzdi = (int)di;
                        int zzgao = (int)gao;

                        if(zzdi==receiveddi && zzgao==receivedgao){    //收到的数据完全正确，则更新那个
                            QByteArray returnba;
                            returnba.resize(2);
                            returnba[0] =temp_data[1];
                            returnba[1] =temp_data[2];
                            //qDebug()<<".........returnba........ b2...."<<returnba.toHex();
                            emit feishiwuswitchstatusreceived(returnba);  //接收到非事务转辙机状态
                            if(this->b2_update_flag==0){
                                auto tempsttr= returnba.toStdString();
                                this->lastba_b2 = QByteArray::fromStdString(tempsttr);
                                 b2_update_flag=1;
                            }else{
                                //更新道岔信息
                                auto zz= lastba_b2.compare(returnba,Qt::CaseInsensitive);
                                if(zz!=0){      //不相等，则通知主窗口，然后更新
                                    this->lastba_b2= QByteArray::fromStdString(returnba.toStdString());
                                    qDebug()<<"非实物转辙机1,3,5,7,9,11,2，4状态已经更新......................."<<this->lastba_b2.toHex();
                                    //emit shouldupdatexinhaojidaochazhuanzeji(lastba_b8);
                                    QString switchs = "1|3|5|7|9|11|2|4";
                                    emit refreshSwitchStatusOfUI(switchs);

                                }else{
                                    QString switchs = "1|3|5|7|9|11|2|4";
                                    this->lastba_b2= QByteArray::fromStdString(returnba.toStdString());
                                    //qDebug()<<"非实物转辙机1,3,5,7,9,11,2轨道区段当前状态......................."<<this->lastba_b2.toHex();
                                     emit refreshSwitchStatusOfUI(switchs);
                                }
                            }
                        }

                    }else{
                        qDebug()<<".........enter concater secner ....";
                    }
                }


                if(temp_arrlist[0]=="b3"){
                    if(temp_arrlist.length()==5){
                        bool ok;
                        QByteArray temp_data;
                        temp_data.resize(3);
                        for(int i=0;i<3;i++){
                            bool ok;
                            int hex =temp_arrlist[i].toInt(&ok,16); //hex=255;ok=true;
                            auto temba = temp_arrlist[i].toLocal8Bit();
                            temp_data[i] =QByteArray::fromHex(temba).front();
                        }
                        //qDebug()<<".........temp_data is ...."<<temp_data.toHex();
                        uint16_t wcrc = Helper::ModbusCRC16(temp_data);
                        unsigned char di=uint8_t(wcrc);
                        unsigned char gao=uint8_t(wcrc>>8);
                        int receiveddi = temp_arrlist[3].toInt(&ok,16);
                        int receivedgao = temp_arrlist[4].toInt(&ok,16);
                        int zzdi = (int)di;
                        int zzgao = (int)gao;
                        if(zzdi==receiveddi && zzgao==receivedgao){    //收到的数据完全正确，则更新那个
                            QByteArray returnba;
                            returnba.resize(2);
                            returnba[0] =temp_data[1];
                            returnba[1] =temp_data[2];
                            //qDebug()<<".........returnba........ b3...."<<returnba.toHex();
                            emit feishiwusignalstatusreceieved(returnba,1);
                        }
                    }else{
                        qDebug()<<".........enter concater secner ....";
                    }

                }

                if(temp_arrlist[0]=="b4"){
                    if(temp_arrlist.length()==5){
                        bool ok;
                        QByteArray temp_data;
                        temp_data.resize(3);
                        for(int i=0;i<3;i++){
                            auto temba = temp_arrlist[i].toLocal8Bit();
                            temp_data[i] =QByteArray::fromHex(temba).front();
                        }
                        uint16_t wcrc = Helper::ModbusCRC16(temp_data);
                        unsigned char di=uint8_t(wcrc);
                        unsigned char gao=uint8_t(wcrc>>8);
                        int receiveddi = temp_arrlist[3].toInt(&ok,16);
                        int receivedgao = temp_arrlist[4].toInt(&ok,16);
                        int zzdi = (int)di;
                        int zzgao = (int)gao;
                        if(zzdi==receiveddi && zzgao==receivedgao){    //收到的数据完全正确，则更新那个
                            QByteArray returnba;
                            returnba.resize(2);
                            returnba[0] =temp_data[1];
                            returnba[1] =temp_data[2];
                            //qDebug()<<".........returnba........ b4...."<<returnba.toHex();
                            emit feishiwusignalstatusreceieved(returnba,2);
                        }
                    }else{
                        qDebug()<<".........enter concater secner ....";
                    }
                }


                if(temp_arrlist[0]=="b5"){
                    if(temp_arrlist.length()==5){
                        bool ok;
                        QByteArray temp_data;
                        temp_data.resize(3);
                        for(int i=0;i<3;i++){
                            auto temba = temp_arrlist[i].toLocal8Bit();
                            temp_data[i] =QByteArray::fromHex(temba).front();
                        }
                        uint16_t wcrc = Helper::ModbusCRC16(temp_data);
                        unsigned char di=uint8_t(wcrc);
                        unsigned char gao=uint8_t(wcrc>>8);
                        int receiveddi = temp_arrlist[3].toInt(&ok,16);
                        int receivedgao = temp_arrlist[4].toInt(&ok,16);
                        int zzdi = (int)di;
                        int zzgao = (int)gao;
                        if(zzdi==receiveddi && zzgao==receivedgao){    //收到的数据完全正确，则更新那个
                            QByteArray returnba;
                            returnba.resize(2);
                            returnba[0] =temp_data[1];
                            returnba[1] =temp_data[2];
                            //qDebug()<<".........returnba........ b5...."<<returnba.toHex();
                            emit feishiwusignalstatusreceieved(returnba,3);
                        }
                    }else{
                        qDebug()<<".........enter concater secner ....";
                    }
                }


                if(temp_arrlist[0]=="b6"){
                    if(temp_arrlist.length()==5){
                        bool ok;
                        QByteArray temp_data;
                        temp_data.resize(3);
                        for(int i=0;i<3;i++){
                            auto temba = temp_arrlist[i].toLocal8Bit();
                            temp_data[i] =QByteArray::fromHex(temba).front();
                        }
                        uint16_t wcrc = Helper::ModbusCRC16(temp_data);
                        unsigned char di=uint8_t(wcrc);
                        unsigned char gao=uint8_t(wcrc>>8);
                        int receiveddi = temp_arrlist[3].toInt(&ok,16);
                        int receivedgao = temp_arrlist[4].toInt(&ok,16);
                        int zzdi = (int)di;
                        int zzgao = (int)gao;
                        if(zzdi==receiveddi && zzgao==receivedgao){    //收到的数据完全正确，则更新那个
                            QByteArray returnba;
                            returnba.resize(2);
                            returnba[0] =temp_data[1];
                            returnba[1] =temp_data[2];
                            //qDebug()<<".........returnba........ b6...."<<returnba.toHex();
                            emit feishiwusignalstatusreceieved(returnba,4);
                        }

                    }else{
                        qDebug()<<".........enter concater secner ....";
                    }

                  }

                if(temp_arrlist[0]=="b7"){
                        if(temp_arrlist.length()==5){
                            bool ok;
                            QByteArray temp_data;
                            temp_data.resize(3);
                            for(int i=0;i<3;i++){
                                auto temba = temp_arrlist[i].toLocal8Bit();
                                temp_data[i] =QByteArray::fromHex(temba).front();
                            }
                            uint16_t wcrc = Helper::ModbusCRC16(temp_data);
                            unsigned char di=uint8_t(wcrc);
                            unsigned char gao=uint8_t(wcrc>>8);
                            int receiveddi = temp_arrlist[3].toInt(&ok,16);
                            int receivedgao = temp_arrlist[4].toInt(&ok,16);
                            int zzdi = (int)di;
                            int zzgao = (int)gao;
                            if(zzdi==receiveddi && zzgao==receivedgao){    //收到的数据完全正确，则更新那个
                                QByteArray returnba;
                                returnba.resize(2);
                                returnba[0] =temp_data[1];
                                returnba[1] =temp_data[2];
                                //qDebug()<<".........returnba........ b7...."<<returnba.toHex();
                                emit feishiwusignalstatusreceieved(returnba,5);
                            }
                        }else{
                            qDebug()<<".........enter concater secner ....";
                        }
                    }

                if(temp_arrlist[0]=="b8"){
                        if(temp_arrlist.length()==5){
                            bool ok;
                            QByteArray temp_data;
                            temp_data.resize(3);
                            for(int i=0;i<3;i++){
                                auto temba = temp_arrlist[i].toLocal8Bit();
                                temp_data[i] =QByteArray::fromHex(temba).front();
                            }
                            uint16_t wcrc = Helper::ModbusCRC16(temp_data);
                            unsigned char di=uint8_t(wcrc);
                            unsigned char gao=uint8_t(wcrc>>8);
                            int receiveddi = temp_arrlist[3].toInt(&ok,16);
                            int receivedgao = temp_arrlist[4].toInt(&ok,16);
                            int zzdi = (int)di;
                            int zzgao = (int)gao;
                            if(zzdi==receiveddi && zzgao==receivedgao){    //收到的数据完全正确，则更新那个
                                QByteArray returnba;
                                returnba.resize(2);
                                returnba[0] =temp_data[1];
                                returnba[1] =temp_data[2];
                                emit guidaoquduancaijistatus(returnba,1);
                                //这里可以先判断一下，然后发送给主窗口，报告轨道上区段变化
                                //这里直接用qbytearray来比较
                                if(this->b8_update_flag==0){
                                    auto tempsttr= returnba.toStdString();
                                    this->lastba_b8 = QByteArray::fromStdString(tempsttr);
                                    qDebug()<<"get done lastba_b8 is ......."<<this->lastba_b8.toHex();
                                    //if 1101G,X1LQG,4DG,1G,3-11G is taken, should redraw the specified segment
                                    b8_update_flag=1;
                                } else{
                                      auto zz= lastba_b8.compare(returnba,Qt::CaseInsensitive);
                                      if(zz!=0){      //不相等，则通知主窗口，然后更新
                                          this->lastba_b8= QByteArray::fromStdString(returnba.toStdString());
                                          qDebug()<<"1101，X1LQG，4DG，11G，3-11G轨道区段变化......................."<<this->lastba_b8.toHex();
                                          emit shouldupdatexinhaojidaochazhuanzeji(lastba_b8);
                                      }else{
                                            //only check 1101G,X1LQG,4DG,1G,3-11G segmentation taken in this area
                                          //this->lastba_b8= QByteArray::fromStdString(returnba.toStdString());
                                          QString segments = "1101G|X1LQG|4DG|3-11G|1G";
                                          this->lastba_b8= QByteArray::fromStdString(returnba.toStdString());
                                          qDebug()<<"1101G，X1LQG，4DG，11G，3-11G轨道区段变化......................."<<this->lastba_b8.toHex();
                                          emit refreshSementStausOfUI(segments);
                                      }
                                }
                            }
                        }else{
                            qDebug()<<".........enter concater secner ....";
                        }

                     }
                if(temp_arrlist[0]=="b9"){
                        if(temp_arrlist.length()==5){
                            bool ok;
                            QByteArray temp_data;
                            temp_data.resize(3);
                            for(int i=0;i<3;i++){
                                bool ok;
                                int hex =temp_arrlist[i].toInt(&ok,16); //hex=255;ok=true;
                                auto temba = temp_arrlist[i].toLocal8Bit();
                                temp_data[i] =QByteArray::fromHex(temba).front();
                            }
                            uint16_t wcrc = Helper::ModbusCRC16(temp_data);
                            unsigned char di=uint8_t(wcrc);
                            unsigned char gao=uint8_t(wcrc>>8);
                            int receiveddi = temp_arrlist[3].toInt(&ok,16);
                            int receivedgao = temp_arrlist[4].toInt(&ok,16);
                            int zzdi = (int)di;
                            int zzgao = (int)gao;
                            if(zzdi==receiveddi && zzgao==receivedgao){    //收到的数据完全正确，则更新那个
                                QByteArray returnba;
                                returnba.resize(2);
                                returnba[0] =temp_data[1];
                                returnba[1] =temp_data[2];
                                //qDebug()<<".........returnba........ b9...."<<returnba.toHex();
                                emit guidaoquduancaijistatus(returnba,2);
                                if(this->b9_update_flag==0){
                                    auto tempsttr= returnba.toStdString();
                                    this->lastba_b9 = QByteArray::fromStdString(tempsttr);
                                    qDebug()<<"get done lastba_b9 is ......."<<this->lastba_b9.toHex();
                                    b9_update_flag=1;
                                } else{
                                      auto zz= lastba_b9.compare(returnba,Qt::CaseInsensitive);
                                      if(zz!=0){      //不相等，则通知主窗口，然后更新
                                         this->lastba_b9= QByteArray::fromStdString(returnba.toStdString());
                                         qDebug()<<"1103，1105，1107轨道区段变化......................."<<this->lastba_b9.toHex();
                                          emit shouldupdatexinhaojidaochazhuanzeji(lastba_b9);
                                      }else{
                                          QString segments = "1107G|1105G|1103G";
                                          this->lastba_b9= QByteArray::fromStdString(returnba.toStdString());
                                          qDebug()<<"1103，1105，1107轨道区段当前状态显示......................."<<this->lastba_b9.toHex();;
                                          emit refreshSementStausOfUI(segments); //if train on track is disappear, we should update scene
                                      }
                                }

                            }
                        }else{
                            qDebug()<<".........enter concater secner ....";
                        }

                     }


                if(temp_arrlist[0]=="b0"){
                        if(temp_arrlist.length()==5){
                            bool ok;
                            QByteArray temp_data;
                            temp_data.resize(3);
                            for(int i=0;i<3;i++){
                                bool ok;
                                int hex =temp_arrlist[i].toInt(&ok,16); //hex=255;ok=true;
                                auto temba = temp_arrlist[i].toLocal8Bit();
                                temp_data[i] =QByteArray::fromHex(temba).front();
                            }
                            uint16_t wcrc = Helper::ModbusCRC16(temp_data);
                            unsigned char di=uint8_t(wcrc);
                            unsigned char gao=uint8_t(wcrc>>8);
                            int receiveddi = temp_arrlist[3].toInt(&ok,16);
                            int receivedgao = temp_arrlist[4].toInt(&ok,16);
                            int zzdi = (int)di;
                            int zzgao = (int)gao;
                            if(zzdi==receiveddi && zzgao==receivedgao){    //收到的数据完全正确，则更新那个
                                QByteArray returnba;
                                returnba.resize(2);
                                returnba[0] =temp_data[1];
                                returnba[1] =temp_data[2];
                                //qDebug()<<".........returnba........"<<returnba.toHex();
                                emit feishiwusignalstatusreceieved(returnba,0);
                            }
                        }else{
                            qDebug()<<".........enter concater secner ....";
                        }
                     }
            }
        }
        const QString request1 = QString::fromUtf8(readData);
        if(request1!=""){
             //qDebug()<<"we are get data from devices and hex is ................."<<readData.toHex('|');
        }
        std::string item;
        bool found = commanditems.try_dequeue(item);
        if(found==true){
            QByteArray baaaaa(item.c_str(),item.length());
            serial.write(baaaaa);
        }
        serial.flush();
        m_mutex.lock();
        if (currentPortName != m_portName) {
            currentPortName = m_portName;
            currentPortNameChanged = true;
        } else {
            currentPortNameChanged = false;
        }
        currentWaitTimeout = m_waitTimeout;
        currentRespone = m_response;
        m_mutex.unlock();
    }
}
