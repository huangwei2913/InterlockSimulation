#include "Tracktologybuilder.h"
#include <QDebug>
#include <QPointF>

TracktologyBuilder::TracktologyBuilder()
{

   this->fromStation="";
   this->toStation="";
   //我们按照原始图来初始化这些道轨状态，然后调用发码命令控制各个区段的颜色（这个地方这里先没有做，到后面考虑之后在做）
     segmentsStatus["3G"]= 0x00;        //3G轨继电器是未被占用的，这些都需要根据列车位置和道岔的正反向状态来变化的，后面再做
     segmentsStatus["1-9G"]= 0x00;     //1-9G轨继电器是未被占用的,但是在进路逻辑进入后，值会改变，1表示占用，0表示未占用，与此同时更新界面上的显示逻辑
     segmentsStatus["IIG"]= 0x00;       //IIG轨继电器是未被占用的
     segmentsStatus["4G"]= 0x00;
//   segmentsStatus["1G"]= 0x00;
//   //由于轨道中既有区段，也有道岔所在区段，都用这个segmentsStatus要区分开来，例如2DG和IIG分别表示2号道岔所在轨道区段，IIG表示正向那个长的区段
     segmentsStatus["2DG"] = 0x00;
     segmentsStatus["4DG"] = 0x00;
//   segmentsStatus["S1LQG"]=0x00;
//   segmentsStatus["1103G"]=0x00;
//   segmentsStatus["1105G"]=0x00;
//   segmentsStatus["1107G"]=0x00;
//   segmentsStatus["1102G"]=0x00;
//   segmentsStatus["1104G"]=0x00;
//   segmentsStatus["1106G"]=0x00;
//   segmentsStatus["1108G"]=0x00;

//    this->trackName2code["3-11G"] = 0;     //在对轨道发码的时候，就会利用下面的数字表示
//    this->trackName2code["1G"] = 1;
//    this->trackName2code["4DG"] = 2;
//    this->trackName2code["X1LQG"] = 3;
//    this->trackName2code["1101G"] = 4;
//    this->trackName2code["1103G"] = 5;
//    this->trackName2code["1105G"] = 6;
//    this->trackName2code["1107G"] = 7;
    this->reseveredPath.clear();
    this->reseveredPathForTheTrain.clear();
    this->trainloc = "";                   //初始化情况下，轨道上都是没车的，发码都是L3
}



QString TracktologyBuilder::judgeTrainInWhichSegementation(){


//    segmentsStatus["3-11G|GFJH"]= ba[1] & 0x01;
//    segmentsStatus["3-11G|GFJQ"]= (ba[1] & 0x02)>>1;
//    segmentsStatus["3-11G|GJQ"]= (ba[1] & 0x04)>>2;
//    segmentsStatus["1G|GFJH"]= (ba[1] & 0x08)>>3;
//    segmentsStatus["1G|GFJQ"]= (ba[1] & 0x10)>>4;
//    segmentsStatus["1G|GJQ"]= (ba[1] & 0x20)>>5;
//    segmentsStatus["4DG|GFJH"]= (ba[1] & 0x40)>>6;
//    segmentsStatus["4DG|GFJQ"]= (ba[1] & 0x80)>>7;
//    segmentsStatus["4DG|GJQ"]= ba[0] & 0x01;
//    segmentsStatus["X1LQG|GFJH"]= (ba[0] & 0x02)>>1;
//    segmentsStatus["X1LQG|GFJQ"]= (ba[0] & 0x04)>>2;
//    segmentsStatus["X1LQG|GJQ"]= (ba[0] & 0x08)>>3;
//    segmentsStatus["1101G|GFJH"]= (ba[0] & 0x10)>>4;
//    segmentsStatus["1101G|GFJQ"]= (ba[0] & 0x20)>>5;
//    segmentsStatus["1101G|GJQ"]= (ba[0] & 0x40)>>6;


    if(this->segmentsStatus.contains("3-11G|GFJH")==true){
       if(this->segmentsStatus["3-11G|GFJH"]== 0x00 && this->segmentsStatus["3-11G|GJQ"]== 0x00 ){
            return "3-11G";
       }
    }


    if(this->segmentsStatus.contains("1G|GFJH")==true){
       if(this->segmentsStatus["1G|GFJQ"]== 0x00 && this->segmentsStatus["1G|GJQ"]== 0x00 ){
            return "1G";
       }
    }

    if(this->segmentsStatus.contains("4DG|GFJH")==true ){
       if(this->segmentsStatus["4DG|GFJQ"]== 0x00   &&   this->segmentsStatus["4DG|GJQ"]== 0x00){
            return "4DG";
       }
    }


    if(this->segmentsStatus.contains("X1LQG|GFJH")==true ){
       if(this->segmentsStatus["X1LQG|GFJH"]== 0x00   &&   this->segmentsStatus["X1LQG|GJQ"]== 0x00){
            return "X1LQG";
       }
    }


    if(this->segmentsStatus.contains("1101G|GFJH")==true ){
       if(this->segmentsStatus["1101G|GFJH"]== 0x00   &&   this->segmentsStatus["1101G|GJQ"]== 0x00){
            return "1101G";
       }
    }

    return "";
}



//在排了进路之后，如果没有车的，就要发这些码出来
//1表示绿灯，3表示红灯，2表示黄灯，0表示灭灯，-1表示未知灯颜色，或者是故障
int TracktologyBuilder::getSingalStatus(QString signal_name){

    qDebug()<<"获取信号机状态TracktologyBuilder::getSingalStatus......";
    //然后讨论非实物信号机，例如
    //QMap<QString, unsigned char>::iterator iter = signalsStatus.begin();

    for (auto iter = signalsStatus.begin(); iter != signalsStatus.end(); ++iter)
        {
            //int aa = *iter;
            //qDebug()<<"the key is"<<iter.key();
            if(iter.key().split("|")[0].contains(signal_name)==true){

//                qDebug()<<signal_name<<"信号机绿灯...."<<
//                          signalsStatus.value(signal_name+"|"+"L",0xFF)
//                        <<"信号机红灯...."<<
//                        signalsStatus.value(signal_name+"|"+"H",0xFF)
//                       <<"信号机黄灯...."<<
//                       signalsStatus.value(signal_name+"|"+"U",0xFF)
//                          ;
                if(signal_name=="S1"){
                    auto tempkey = signal_name+"|"+"L";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 1;
                    }
                    tempkey = signal_name+"|"+"H";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 3;
                    }
                    tempkey = signal_name+"|"+"U";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 2;
                    }
                }
                else if(signal_name=="S3"){



                    auto tempkey = signal_name+"|"+"L";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 1;
                    }
                    tempkey = signal_name+"|"+"H";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 3;
                    }
                    tempkey = signal_name+"|"+"U";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 2;
                    }
                }
                else if(signal_name=="SII"){
                    qDebug()<<"应该运行到这里啊,MBBBBB..............";
                    auto tempkey = signal_name+"|"+"L";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 1;
                    }
                    tempkey = signal_name+"|"+"H";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 3;
                    }
                    tempkey = signal_name+"|"+"U";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 2;
                    }
                }

                else if(signal_name=="S4"){

                    auto tempkey = signal_name+"|"+"L";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 1;
                    }
                    tempkey = signal_name+"|"+"H";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 3;
                    }
                    tempkey = signal_name+"|"+"U";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 2;
                    }
                }

                else if(signal_name=="XII"){

                    auto tempkey = signal_name+"|"+"L";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 1;
                    }
                    tempkey = signal_name+"|"+"H";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 3;
                    }

                }

                else if(signal_name=="1103"){

                    auto tempkey = signal_name+"|"+"L";
                    qDebug()<<".................MDDDDDBBBBBBBBBBBBB......."<<signalsStatus.value(tempkey,0xFF);
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 1;
                    }
                    tempkey = signal_name+"|"+"H";
                    qDebug()<<".................MDDDDDBBBBBBBBBBxxxxxx......."<<signalsStatus.value(tempkey,0xFF);
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 3;
                    }
                    tempkey = signal_name+"|"+"U";
                    qDebug()<<".................MDDDDDBBBBBBBBBBzzzz......."<<signalsStatus.value(tempkey,0xFF);
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 2;
                    }

                }

                else if(signal_name=="1105"){

                    auto tempkey = signal_name+"|"+"L";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 1;
                    }
                    tempkey = signal_name+"|"+"H";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 3;
                    }
                    tempkey = signal_name+"|"+"U";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 2;
                    }

                }


                else if(signal_name=="1107"){

                    auto tempkey = signal_name+"|"+"L";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 1;
                    }
                    tempkey = signal_name+"|"+"H";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 3;
                    }
                    tempkey = signal_name+"|"+"U";
                    if(signalsStatus.value(tempkey,0xFF)==0x00){
                      return 2;
                    }

                }



                else{
                    continue;
                }
            }


        }
        return -1;
}

int TracktologyBuilder::getSwitchStatus(QString switch_name){

   // QMap<QString, unsigned char>::iterator iter = switchsStatus.begin();
    for (auto iter = switchsStatus.begin(); iter != switchsStatus.end(); ++iter){
        //qDebug()<<"道岔状态检查........"<<iter.key();
            if(iter.key().split("|")[0]==switch_name){
               auto tempKey = iter.key().split("|")[0] +"|"+"DB";
               if(switchsStatus[tempKey]==0x00){    //表示定向
                   return 0;
               }else{
                   auto tempKey1 = iter.key().split("|")[0] +"|"+"FB";
                   if(switchsStatus[tempKey1]==0x00){
                       return 1;                   //表示反向
                   }
               }
        }

    }
    return -1;    //表示状态未知
}


//例如输入3-11G,在这里判断其状态,1表示是占用，-1表示未知，0表示未占用，我好像知道了
int TracktologyBuilder::getSegmentStatus(QString segment_name){

    if(segment_name=="3-11G"){
        qDebug()<<"检查3-11G状态，判断是否轨道被占用..........";
      //判断的
        if(segmentsStatus.contains(segment_name+"|"+"GJQ")==false){
            return -1;   //没有找到
        }

        qDebug()<<"存在.........."<<segment_name+"|"+"GJQ"<<"...."<<segment_name+"|"+"GFJH";
        qDebug()<<"值等于.........."<<segmentsStatus[segment_name+"|"+"GJQ"]
                                    << segmentsStatus[segment_name+"|"+"GFJH"];
       if(segmentsStatus[segment_name+"|"+"GJQ"]==0x00 &&
               segmentsStatus[segment_name+"|"+"GFJH"]==0x00){
           //qDebug()<<"当"<<segment_name+"|"+"GJQ"<<"等于1并且"<<segment_name+"|"+"GFJH"<<"等于0的时候表示定向？？？";
           return 0;   //未占用
       }
       if(segmentsStatus[segment_name+"|"+"GJQ"]==0x01 &&
               segmentsStatus[segment_name+"|"+"GFJQ"]==0x01){
           return 1;  //占用
       }
      return -1;
    }

    if(segment_name=="1G"){
        //这里要进行判断
        if(segmentsStatus.contains(segment_name+"|"+"GJQ")==false){
            return -1;   //没有找到
        }
        qDebug()<<"存在.........."<<segment_name+"|"+"GFJH"<<"...."<<segment_name+"|"+"GFJQ"<<segment_name+"|"+"GJQ";
        qDebug()<<"值等于.........."<<segmentsStatus[segment_name+"|"+"GFJH"]
                                    << segmentsStatus[segment_name+"|"+"GFJQ"]
                                    << segmentsStatus[segment_name+"|"+"GJQ"];


        if(segmentsStatus[segment_name+"|"+"|GFJH"]==0x00 &&
                segmentsStatus[segment_name+"|"+"GFJQ"]==0x01 && segmentsStatus[segment_name+"|"+"GJQ"]==0x01 ){
            qDebug()<<"1G已经占用..........";
            return 1;  //已占用
        }else{
            qDebug()<<"1G未占用............";
            return 0;  //已占用
        }

//        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x00 &&
//                segmentsStatus[segment_name+"|"+"GFJH"]==0x00){
//            return 0;   //未被占用
//        }

       return -1;
    }



    if(segment_name=="4DG"){
        //这里要进行判断
        if(segmentsStatus.contains(segment_name+"|"+"GJQ")==false){
            return -1;   //没有找到
        }
        qDebug()<<"存在.........."<<segment_name+"|"+"GJQ"<<"...."<<segment_name+"|"+"GFJH";
        qDebug()<<"值等于.........."<<segmentsStatus[segment_name+"|"+"GJQ"]
                                    << segmentsStatus[segment_name+"|"+"GFJH"];
        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x00 &&
                segmentsStatus[segment_name+"|"+"GFJH"]==0x00){
            return 0;   //未被占用
        }

        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x01 &&
                segmentsStatus[segment_name+"|"+"GFJQ"]==0x01){
            return 1;  //已占用
        }
       return -1;
    }



    if(segment_name=="X1LQG"){
        //这里要进行判断
        if(segmentsStatus.contains(segment_name+"|"+"GJQ")==false){
            return -1;   //没有找到
        }
        qDebug()<<"存在.........."<<segment_name+"|"+"GJQ"<<"...."<<segment_name+"|"+"GFJH";
        qDebug()<<"值等于.........."<<segmentsStatus[segment_name+"|"+"GJQ"]
                                    << segmentsStatus[segment_name+"|"+"GFJH"];
        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x00 &&
                segmentsStatus[segment_name+"|"+"GFJH"]==0x00){
            return 0;   //未被占用
        }

        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x01 &&
                segmentsStatus[segment_name+"|"+"GFJQ"]==0x01){
            return 1;  //已占用
        }
       return -1;
    }


    if(segment_name=="1101G"){
        //这里要进行判断
        if(segmentsStatus.contains(segment_name+"|"+"GJQ")==false){
            return -1;   //没有找到
        }
        qDebug()<<"存在.........."<<segment_name+"|"+"GJQ"<<"...."<<segment_name+"|"+"GFJH"<<segment_name+"|"+"GFJQ";;
        qDebug()<<"值等于.........."<<segmentsStatus[segment_name+"|"+"GJQ"]
                                    << segmentsStatus[segment_name+"|"+"GFJH"]
                                    << segmentsStatus[segment_name+"|"+"GFJQ"];
        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x00 &&
                segmentsStatus[segment_name+"|"+"GFJH"]==0x00 &&  segmentsStatus[segment_name+"|"+"GFJQ"]==0x01){
            qDebug()<<"1101G未被占用";
            return 0;   //未被占用
        }else{
            qDebug()<<"1101G已被占用";
          return 1;  //已占用
        }

//        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x01 &&
//                segmentsStatus[segment_name+"|"+"GFJQ"]==0x01){
//              qDebug()<<"1101G已被占用";
//            return 1;  //已占用
//        }
       return -1;
    }


    if(segment_name=="1103G"){
        //这里要进行判断
        if(segmentsStatus.contains(segment_name+"|"+"GJQ")==false){
            return -1;   //没有找到
        }
        qDebug()<<"存在.........."<<segment_name+"|"+"GJQ"<<"...."<<segment_name+"|"+"GFJH";
        qDebug()<<"值等于.........."<<segmentsStatus[segment_name+"|"+"GJQ"]
                                    << segmentsStatus[segment_name+"|"+"GFJH"];
        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x00 &&
                segmentsStatus[segment_name+"|"+"GFJH"]==0x00  ){
            qDebug()<<"1103G没有被占用";
            return 0;   //未被占用
        }

        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x01 &&
                segmentsStatus[segment_name+"|"+"GFJH"]==0x00){
            return 1;  //已占用
        }
       return -1;
    }



    if(segment_name=="1105G"){
        //这里要进行判断
        if(segmentsStatus.contains(segment_name+"|"+"GJQ")==false){
            return -1;   //没有找到
        }
        qDebug()<<"存在.........."<<segment_name+"|"+"GJQ"<<"...."<<segment_name+"|"+"GFJH"<<segment_name+"|"+"GFJQ";
        qDebug()<<"值等于.........."<<segmentsStatus[segment_name+"|"+"GJQ"]
                                    << segmentsStatus[segment_name+"|"+"GFJH"]
                                    <<segmentsStatus[segment_name+"|"+"GFJQ"];
        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x00 &&
                segmentsStatus[segment_name+"|"+"GFJH"]==0x00 && segmentsStatus[segment_name+"|"+"GFJQ"]==0x01){
            qDebug()<<"1105G未被占用.....";
            return 0;   //未被占用
        }else{
            qDebug()<<"1105G已占用.....";
            return 1;  //已占用
        }

//        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x01 &&
//                segmentsStatus[segment_name+"|"+"GFJH"]==0x00){
//             qDebug()<<"1105G已占用.....";
//            return 1;  //已占用
//        }
       return -1;
    }


    if(segment_name=="1107G"){
        //这里要进行判断
        if(segmentsStatus.contains(segment_name+"|"+"GJQ")==false){
            return -1;   //没有找到
        }
        qDebug()<<"存在.........."<<segment_name+"|"+"GJQ"<<"...."<<segment_name+"|"+"GFJH";
        qDebug()<<"值等于.........."<<segmentsStatus[segment_name+"|"+"GJQ"]
                                    << segmentsStatus[segment_name+"|"+"GFJH"];
        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x00 &&
                segmentsStatus[segment_name+"|"+"GFJH"]==0x00){
            qDebug()<<"1107没有占用..............";
            return 0;   //未被占用
        }

        if(segmentsStatus[segment_name+"|"+"GJQ"]==0x01 &&
                segmentsStatus[segment_name+"|"+"GFJH"]==0x00){    // is the judgement correts ?? GFJQ--->GFJH
            qDebug()<<"1107已经被占用..............";
            return 1;  //已占用
        }
       return -1;
    }






    if(segment_name=="3G"){         //这些都是虚拟道轨
        qDebug()<<"默认情况下3G发返回的是没有占用...........。这种默认的情况也要存储和更新....";
        if(segmentsStatus.contains(segment_name)==true){
            return segmentsStatus[segment_name];
        }
    }

    if(segment_name=="1-9G"){      //这些都是虚拟道轨
        if(segmentsStatus.contains(segment_name)==true){
            return segmentsStatus[segment_name];
        }
    }


    if(segment_name=="IIG"){      //这些都是虚拟道轨
        if(segmentsStatus.contains(segment_name)==true){
            return segmentsStatus[segment_name];
        }
    }


    if(segment_name=="4G"){      //这些都是虚拟道轨
        if(segmentsStatus.contains(segment_name)==true){
            return segmentsStatus[segment_name];
        }
    }


    if(segment_name=="2DG"){      //这些都是虚拟道轨
        if(segmentsStatus.contains(segment_name)==true){
            return segmentsStatus[segment_name];
        }
    }


    return -1;      //未知状态
}

void TracktologyBuilder::addCriticalPointsInformation(QString pointName, QPointF loc){
 criticalPointsinToplogy.insert(pointName,loc);
}

//这一块需要确认的 ""代表灭灯
void TracktologyBuilder::updateCijiaXinhaoji(){


    qDebug()<<"实物信号机XI和1101状态...."
            <<shiwusignalswithcStatus["X1LXJ"]
            <<shiwusignalswithcStatus["X1LJ"]
            <<shiwusignalswithcStatus["X1LUJ"]
            <<shiwusignalswithcStatus["X1DJ"];

    //LXJ无效点红灯，LXJ有效LJ有效点绿灯，LXJ有效LJ无效LUJ有效点黄灯，LXJ与LUJ有效点绿黄灯

    if(shiwusignalswithcStatus["X1DJ"]== 0x00){//LXJ无效点红灯
        cijiaSignalStatus["XI-B1"]="red";
        cijiaSignalStatus["XI-B2"]="";

    }
    if(shiwusignalswithcStatus["X1LXJ"]==0x00 && shiwusignalswithcStatus["X1LUJ"]==0x00 && shiwusignalswithcStatus["X1DJ"]==0x00){ //LXJ有效LJ无效LUJ有效点黄灯
        cijiaSignalStatus["XI-B1"]="yellow";
        cijiaSignalStatus["XI-B2"]="green";
    }

    if(shiwusignalswithcStatus["X1LXJ"]==0x00 && shiwusignalswithcStatus["X1LJ"]==0x00 && shiwusignalswithcStatus["X1DJ"]==0x00){
        cijiaSignalStatus["XI-B2"]="green";
        cijiaSignalStatus["XI-B1"]="";
    }

    if(shiwusignalswithcStatus["X1LXJ"]==0x00 && shiwusignalswithcStatus["X1DJ"]==0x00 && shiwusignalswithcStatus["X1LJ"]==0x01 && shiwusignalswithcStatus["X1LUJ"]==0x01){
        cijiaSignalStatus["XI-B2"]="yellow";
        cijiaSignalStatus["XI-B1"]="";
    }

//    if(shiwusignalswithcStatus["X1LXJ"]==0x00 && shiwusignalswithcStatus["X1DJ"]==0x00){ //LXJ有效LJ有效点绿灯
//        cijiaSignalStatus["XI-B2"]="yellow";
//        cijiaSignalStatus["XI-B1"]="red";

//    }

//    if(shiwusignalswithcStatus["X1LXJ"]==0x00 && shiwusignalswithcStatus["X1LJ"]==0x00 && shiwusignalswithcStatus["X1LUJ"]==0x00&& shiwusignalswithcStatus["X1DJ"]==0x00){ //LXJ有效LJ有效点绿灯
//        cijiaSignalStatus["XI-B2"]="green";
//        cijiaSignalStatus["XI-B1"]="red";
//    }


}

void TracktologyBuilder::updateshiwuxinhaojihedaochaStatus(QByteArray ba){
    if(ba.length()!=2){
        return;
    }
    //LXJ无效点红灯，LXJ有效LJ有效点绿灯，LXJ有效LJ无效LUJ有效点黄灯，LXJ与LUJ有效点绿黄灯
    this->shiwusignalswithcStatus["X1LXJ"] = ba[1]&0x01;
    this->shiwusignalswithcStatus["X1LJ"] = (ba[1]&0x02)>>1;
    this->shiwusignalswithcStatus["X1LUJ"] =  (ba[1]&0x04)>>2;
    this->shiwusignalswithcStatus["X1DJ"] =  (ba[1]&0x08)>>3;
    this->shiwusignalswithcStatus["1101DJ"] = (ba[1]&0x10)>>4;
    this->shiwusignalswithcStatus["11012DJ"] =(ba[1]&0x20)>>5;
    this->shiwusignalswithcStatus["1101LJ"] =(ba[1]&0x40)>>6;
    this->shiwusignalswithcStatus["1101HJ"] =(ba[1]&0x80)>>7;
    this->shiwusignalswithcStatus["1101UJ"] = ba[0]&0x01;
    this->shiwusignalswithcStatus["1101LUJ"] =(ba[0]&0x02)>>1;
    updateCijiaXinhaoji();  // i suspect this function call is not necessary
}

void TracktologyBuilder::cacelRouteStartpointHanler(QString item_name){
    if(item_name==""){
        return;
    }
    this->fromStation="";  //直接恢复初始选择
    this->toStation="";
}

//从TopScene进路请求发出的
void TracktologyBuilder::requestRouteHandler(QString item_name){
    qDebug()<<"xxxxx.............requestRouteHandler....."<<item_name;
   if(item_name==""){
       return;
   }
   if(!item_name.contains("-")){
        return;
   }
   if( this->fromStation==""){
       this->fromStation=item_name.split("-")[0];
   }
   else{
      this->toStation =item_name.split("-")[0];
   }
   qDebug()<<"xxxxx.............from station"<<  this->fromStation;
   qDebug()<<"xxxxx.............to station"<<  this->toStation ;
   if(this->toStation==""){
    return;
   }
   //在这里当确定好进路之后，开始进入连锁控制逻辑
   emit addRouteRequest(this->fromStation,this->toStation);

}

void TracktologyBuilder::updateouteSelctionInfoFrom(QString fromwhere){
    this->fromStation =fromwhere;
    qDebug()<<"xxxxx.............from station"<<  this->fromStation;
}


void TracktologyBuilder::updateouteSelctionInfoto(QString towhere){
    this->toStation =towhere;
    qDebug()<<"xxxxx.............to station"<<  this->toStation;

}


void TracktologyBuilder::addItemToMap(QString &item_name, QString &item_location)
{
    item_location_map.insert(item_name,item_location);
}


//判断是否可以保留改线路,如果可以保留保留则发送回复该信息出去，
void TracktologyBuilder::ReserverRoutePath(QString trainNo, QString startx, QString endx)
{

    //发码在最后搞？？？
    reseveredPathForTheTrain.insert(trainNo,startx+">"+endx);
    reseveredPath.append(startx+">"+endx);
    //在排列进路之后触发，发码， 3-11DG，下行进站信号机，X，反方向进站信号机SN,在这之间的叫站内区段。
    //然而X1LQG，1101G，1103G，1105G，1107G属于区间的，这些区间区段都可以根据车子在位置来发码,
    //3-11DG,，例如X进站机开放信号了，才会发码。 X1发车之后，4DG才能发JC码之后的其它码序
    //没有进路的时候，需要发 JC码
    //只要是没有车占用的情况，就发出L3码，第一种情况
    //第一种情况，有车占用1G,根据车的位置调整码序变化，发码只是代表频率变化，从后面开始变化
    //对于跨段的情况下，按照占用来表示
    //先针对区间的，根据车的位置来进行，车的位置。信号机显示红灯，信号机给它信息，轨道电路发送响应的低频信息，
    //前方信号机是红黄码， 如果是信号机是开放了，信号机，根据前方空闲情况来定

    //我们首先针对区间的X1LQG区段发码，先不考虑跨轨的情况
//    if(this->trainloc=="1G"){ //有车占用1轨情况
//    }


    //根据出战信号机XI显示的红色来决定3-11G 1G都是红黄码




}


void TracktologyBuilder::clearRoutePath(QString trainNo){

    auto zz = reseveredPathForTheTrain.take(trainNo);
    reseveredPath.removeOne(zz);

}


void TracktologyBuilder::updatekeepSegmentstatus(QByteArray ba,int classno){

    if(ba.length()!=2){
        return;
    }

    if (classno==1){

        segmentsStatus["3-11G|GFJH"]= ba[1] & 0x01;
        segmentsStatus["3-11G|GFJQ"]= (ba[1] & 0x02)>>1;
        segmentsStatus["3-11G|GJQ"]= (ba[1] & 0x04)>>2;
        segmentsStatus["1G|GFJH"]= (ba[1] & 0x08)>>3;
        segmentsStatus["1G|GFJQ"]= (ba[1] & 0x10)>>4;
        segmentsStatus["1G|GJQ"]= (ba[1] & 0x20)>>5;
        segmentsStatus["4DG|GFJH"]= (ba[1] & 0x40)>>6;
        segmentsStatus["4DG|GFJQ"]= (ba[1] & 0x80)>>7;
        segmentsStatus["4DG|GJQ"]= ba[0] & 0x01;
        segmentsStatus["X1LQG|GFJH"]= (ba[0] & 0x02)>>1;
        segmentsStatus["X1LQG|GFJQ"]= (ba[0] & 0x04)>>2;
        segmentsStatus["X1LQG|GJQ"]= (ba[0] & 0x08)>>3;
        segmentsStatus["1101G|GFJH"]= (ba[0] & 0x10)>>4;
        segmentsStatus["1101G|GFJQ"]= (ba[0] & 0x20)>>5;
        segmentsStatus["1101G|GJQ"]= (ba[0] & 0x40)>>6;
        qDebug()<<"3-11G,1G,4DG,X1LQG,1101G状态已经更新=......."<<
                                    segmentsStatus["3-11G|GFJH"]<<
                                    segmentsStatus["3-11G|GFJQ"]<<
                                    segmentsStatus["3-11G|GJQ"]<<
                                    segmentsStatus["1G|GFJH"]<<
                                    segmentsStatus["1G|GFJQ"]<<
                                    segmentsStatus["1G|GJQ"]<<
                                    segmentsStatus["4DG|GFJH"]<<
                                    segmentsStatus["4DG|GFJQ"]<<
                                    segmentsStatus["4DG|GJQ"]<<
                                    segmentsStatus["X1LQG|GFJH"]<<
                                    segmentsStatus["X1LQG|GFJQ"]<<
                                    segmentsStatus["X1LQG|GJQ"]<<
                                    segmentsStatus["1101G|GFJH"]<<
                                    segmentsStatus["1101G|GFJQ"]<<
                                    segmentsStatus["1101G|GJQ"];


    }else if (classno==2){

        segmentsStatus["1103G|GFJH"]= ba[1] & 0x01;
        segmentsStatus["1103G|GFJQ"]= (ba[1] & 0x02)>>1;
        segmentsStatus["1103G|GJQ"]= (ba[1] & 0x04)>>2;
        segmentsStatus["1105G|GFJH"]= (ba[1] & 0x08)>>3;
        segmentsStatus["1105G|GFJQ"]= (ba[1] & 0x10)>>4;
        segmentsStatus["1105G|GJQ"]= (ba[1] & 0x20)>>5;
        segmentsStatus["1107G|GFJH"]= (ba[1] & 0x40)>>6;
        segmentsStatus["1107G|GFJQ"]= (ba[1] & 0x80)>>7;
        segmentsStatus["1107G|GJQ"]= ba[0] & 0x01;

        qDebug()<<"1103G,1105G,1107G状态已经更新..............................."<<
                      segmentsStatus["1103G|GFJH"]<<
                      segmentsStatus["1103G|GFJQ"]<<
                      segmentsStatus["1103G|GJQ"]<<
                      segmentsStatus["1105G|GFJH"]<<
                      segmentsStatus["1105G|GFJQ"]<<
                      segmentsStatus["1105G|GJQ"]<<
                      segmentsStatus["1107G|GFJH"]<<
                      segmentsStatus["1107G|GFJQ"]<<
                      segmentsStatus["1107G|GJQ"];

    }


    else{
      return;
    }


}

void TracktologyBuilder::updatekeepSignalStatus(QByteArray ba,int classno){
  // qDebug()<<" entering in this updatekeepSignalStatus classno =......."<<classno;
  if (ba.length()!=2){
      return;
  }
  if (classno==1){   //0xb3
        //qDebug()<<"outehigh entering in this section......."<<ba.toHex();
        //默认采用低电平0有效,
        signalsStatus["XN|2U"] = ba[0] & 0x01;
        signalsStatus["XN|YB"] = (ba[0] & 0x02)>>1;
        signalsStatus["S1|L"] = (ba[0] & 0x04)>>2;
        signalsStatus["S1|H"] =  (ba[0] & 0x08)>>3;
        signalsStatus["S1|U"] =  (ba[0] & 0x10)>>4;
        signalsStatus["S1|FB"] = (ba[0] & 0x20)>>5;
        signalsStatus["X|U"] =   ba[1] & 0x01;
        signalsStatus["X|L"] =  (ba[1] & 0x02)>>1;
        signalsStatus["X|H"] =  (ba[1] & 0x04)>>2;
        signalsStatus["X|2U"] = (ba[1] & 0x08)>>3;
        signalsStatus["X|YB"] = (ba[1] & 0x10)>>4;
        signalsStatus["XN|U"] = (ba[1] & 0x20)>>5;
        signalsStatus["XN|L"] = (ba[1] & 0x40)>>6;
        signalsStatus["XN|H"] = (ba[1] & 0x80)>>7;

        qDebug()<<"信号机颜色信息XN,S1,X........."
                  <<signalsStatus["XN|2U"]
                  <<signalsStatus["XN|YB"]
                  <<signalsStatus["S1|L"]
                  <<signalsStatus["S1|H"]
                  <<signalsStatus["S1|U"]
                  <<signalsStatus["S1|FB"]
                  <<signalsStatus["X|U"]
                  <<signalsStatus["X|L"]
                  <<signalsStatus["X|H"]
                  <<signalsStatus["X|2U"]
                  <<signalsStatus["X|YB"]
                  <<signalsStatus["XN|U"]
                  <<signalsStatus["XN|L"]
                  <<signalsStatus["XN|H"] ;




  }else if (classno==2){ //0Xb4

      signalsStatus["S4|L"] = ba[0] & 0x01;
      signalsStatus["S4|H"] = (ba[0] & 0x02)>>1;
      signalsStatus["S4|U"] = (ba[0] & 0x04)>>2;
      signalsStatus["S4|FB"] = (ba[0] & 0x08)>>3;
      signalsStatus["XII|L"] = (ba[0] & 0x10)>>4;
      signalsStatus["XII|H"] = (ba[0] & 0x20)>>5;
      signalsStatus["SII|L"] = ba[1] & 0x01;
      signalsStatus["SII|H"] = (ba[1] & 0x02)>>1;
      signalsStatus["SII|U"] = (ba[1] & 0x04)>>2;
      signalsStatus["SII|FB"] = (ba[1] & 0x08)>>3;
      signalsStatus["S3|L"] = (ba[1] & 0x10)>>4;
      signalsStatus["S3|H"] = (ba[1] & 0x20)>>5;
      signalsStatus["S3|U"] = (ba[1] & 0x40)>>6;
      signalsStatus["S3|FB"] = (ba[1] & 0x80)>>7;
      //根据这些状态来更新界面中的信号灯颜色，是必须的
      qDebug()<<"信号机颜色信息S4,XII,SII........."<<signalsStatus["S4|L"]
                                    <<signalsStatus["S4|H"]
                                    <<signalsStatus["S4|U"]
                                    <<signalsStatus["S4|FB"]
                                    <<signalsStatus["XII|L"]
                                    <<signalsStatus["XII|H"]
                                    <<signalsStatus["SII|L"]
                                    <<signalsStatus["SII|H"]
                                    <<signalsStatus["SII|U"]
                                    <<signalsStatus["SII|FB"]
                                    <<signalsStatus["S3|L"]
                                    <<signalsStatus["S3|H"]
                                    <<signalsStatus["S3|U"]
                                    <<signalsStatus["S3|FB"];
  }
  else if (classno==3){ //0Xb5

      signalsStatus["SN|2U"] = ba[0] & 0x01;
      signalsStatus["SN|YB"] = (ba[0] & 0x02)>>1;
      signalsStatus["X3|L"] = (ba[0] & 0x04)>>2;
      signalsStatus["X3|H"] = (ba[0] & 0x08)>>3;
      signalsStatus["X3|U"] = (ba[0] & 0x10)>>4;
      signalsStatus["X4|L"] = (ba[0] & 0x20)>>5;
      signalsStatus["X4|H"] = (ba[0] & 0x40)>>6;

      signalsStatus["S|U"] = ba[1] & 0x01;
      signalsStatus["S|L"] = (ba[1] & 0x02)>>1;
      signalsStatus["S|H"] = (ba[1] & 0x04)>>2;
      signalsStatus["S|2U"] = (ba[1] & 0x08)>>3;
      signalsStatus["S|YB"] = (ba[1] & 0x10)>>4;
      signalsStatus["SN|U"] = (ba[1] & 0x20)>>5;
      signalsStatus["SN|L"] = (ba[1] & 0x40)>>6;
      signalsStatus["SN|H"] = (ba[1] & 0x80)>>7;

      qDebug()<<"信号机颜色信息SN,X3,X4,S.............."<<signalsStatus["SN|2U"]
                                    <<signalsStatus["SN|YB"]
                                    <<signalsStatus["X3|L"]
                                    <<signalsStatus["X3|H"]
                                    <<signalsStatus["X3|U"]
                                    <<signalsStatus["X4|L"]
                                    <<signalsStatus["X4|H"]
                                    <<signalsStatus["S|U"]
                                    <<signalsStatus["S|L"]
                                    <<signalsStatus["S|H"]
                                    <<signalsStatus["S|2U"]
                                    <<signalsStatus["S|YB"]
                                    <<signalsStatus["SN|U"]
                                    <<signalsStatus["SN|L"]
                                    <<signalsStatus["SN|H"];


  }
  else if (classno==4){ //0Xb6

      signalsStatus["1107|L"] = ba[0] & 0x01;
      signalsStatus["1107|H"] = (ba[0] & 0x02)>>1;
      signalsStatus["1107|U"] = (ba[0] & 0x04)>>2;
      signalsStatus["1103|L"] = ba[1] & 0x01;
      signalsStatus["1103|H"] = (ba[1] & 0x02)>>1;
      signalsStatus["1103|U"] = (ba[1] & 0x04)>>2;
      signalsStatus["1105|L"] = (ba[1] & 0x08)>>3;
      signalsStatus["1105|H"] = (ba[1] & 0x10)>>4;
      signalsStatus["1105|U"] = (ba[1] & 0x20)>>5;

      qDebug()<<"信号机颜色信息1107,1103,1105............."
            <<   signalsStatus["1107|L"]
       << signalsStatus["1107|H"]
       << signalsStatus["1107|U"]
       << signalsStatus["1103|L"]
       << signalsStatus["1103|H"]
      <<  signalsStatus["1103|U"]
      <<  signalsStatus["1105|L"]
       << signalsStatus["1105|H"]
      <<  signalsStatus["1105|U"] ;



  }
  else if (classno==5){//0Xb7
      signalsStatus["1106|L"] = ba[0] & 0x01;
      signalsStatus["1106|H"] = (ba[0] & 0x02)>>1;
      signalsStatus["1106|U"] = (ba[0] & 0x04)>>2;
      signalsStatus["1108|L"] = (ba[0] & 0x08)>>3;
      signalsStatus["1108|H"] = (ba[0] & 0x10)>>4;
      signalsStatus["1108|U"] = (ba[0] & 0x20)>>5;

      signalsStatus["1102|L"] = ba[1] & 0x01;
      signalsStatus["1102|H"] = (ba[1] & 0x02)>>1;
      signalsStatus["1102|U"] = (ba[1] & 0x04)>>2;
      signalsStatus["1104|L"] = (ba[1] & 0x08)>>3;
      signalsStatus["1104|H"] = (ba[1] & 0x10)>>4;
      signalsStatus["1104|U"] = (ba[1] & 0x20)>>5;

  }else if(classno==0){   //oxb0
      this->switchsStatus["4|DB"] = ba[1] & 0x01;       //表示4号道岔定标
      this->switchsStatus["4|FB"] = (ba[1] & 0x02)>>1;   //表示4号道岔反标
      this->switchsStatus["4|DBJ"] = (ba[1] & 0x04)>>2;       //表示4号道岔定标
      this->switchsStatus["4|FBJ"] = (ba[1] & 0x08)>>3;   //表示4号道岔反标
      qDebug()<<"4号道岔定标反标状态已更新....."<<this->switchsStatus["4|DB"]<<this->switchsStatus["4|FB"]<<this->switchsStatus["4|DBJ"]<<this->switchsStatus["4|FBJ"];

  }else if(classno==9){     //采集第一块板子的时候

      //当采集第一块板的时候,0x91时   //控制X1和1101信号机 //控制X1和1101信号机
    signalsStatus["X1|LXJ"] = ba[1] & 0x01;
    signalsStatus["X1|LJ"] = (ba[1] & 0x02)>>1;
    signalsStatus["X1|LUJ"] = (ba[1] & 0x04)>>2;
    signalsStatus["X1|DJ"] = (ba[1] & 0x08)>>3;
    signalsStatus["1101|DJ"] =(ba[1] & 0x10)>>4;
    signalsStatus["1101|2DJ"] = (ba[1] & 0x20)>>5;
    signalsStatus["1101|LJ"] = (ba[1] & 0x40)>>6;
    signalsStatus["1101|HJ"] = (ba[1] & 0x80)>>7;
    signalsStatus["1101|UJ"] = ba[0] & 0x01;
    signalsStatus["1101|LUJ"] =(ba[0] & 0x02)>>1;
    signalsStatus["X1|L"] =(ba[0] & 0x04)>>2;
    signalsStatus["X1|U"] =(ba[0] & 0x08)>>3;
    signalsStatus["X1|H"] =(ba[0] & 0x10)>>4;
    signalsStatus["1101|L"] =(ba[0] & 0x20)>>5;
    signalsStatus["1101|U"] =(ba[0] & 0x04)>>6;
    signalsStatus["1101|H"] =(ba[0] & 0x08)>>7;

    //X1居然是实物信号机
    //判断X1
    qDebug()<<"XI,1101实物信号机状态 ................."
    <<    signalsStatus["X1|LXJ"] <<
    signalsStatus["X1|LJ"] <<
    signalsStatus["X1|LUJ"] <<
    signalsStatus["X1|DJ"] <<
    signalsStatus["1101|DJ"] <<
    signalsStatus["1101|2DJ"] <<
    signalsStatus["1101|LJ"] <<
    signalsStatus["1101|HJ"] <<
    signalsStatus["1101|UJ"] <<
    signalsStatus["1101|LUJ"]<<
    signalsStatus["X1|L"] <<
    signalsStatus["X1|U"] <<
    signalsStatus["X1|H"]<<
    signalsStatus["1101|L"]<<
    signalsStatus["1101|U"]<<
    signalsStatus["1101|H"] ;

    updateshiwuxinhaojihedaochaStatus(ba);

  }
  else{
      return;
  }
}

//非实物转辙机
void TracktologyBuilder::updatekeepSwitchStatus(QByteArray ba){

    if (ba.length()!=2){
        return;
    }

    switchsStatus["1|DB"]=ba[1]& 0x01;
    switchsStatus["1|FB"]=(ba[1]& 0x02)>>1;
    switchsStatus["3|DB"]=(ba[1]& 0x04)>>2;
    switchsStatus["3|FB"]=(ba[1]& 0x08)>>3;
    switchsStatus["5|DB"]=(ba[1]& 0x10)>>4;
    switchsStatus["5|FB"]=(ba[1]& 0x20)>>5;
    switchsStatus["7|DB"]=(ba[1]& 0x40)>>6;
    switchsStatus["7|FB"]=(ba[1]& 0x80)>>7;
    switchsStatus["9|DB"]=ba[0]& 0x01;
    switchsStatus["9|FB"]=(ba[0]& 0x02)>>1;
    switchsStatus["11|DB"]=(ba[0]& 0x04)>>2;
    switchsStatus["11|FB"]= (ba[0]& 0x08)>>3;
    switchsStatus["2|DB"]=  (ba[0]& 0x10)>>4;
    switchsStatus["2|FB"]=  (ba[0]& 0x20)>>5;

    qDebug()<<"在非实物转辙机状态1,3,5,7,9,11,2已经更新............................."
        <<switchsStatus["1|DB"]
        <<switchsStatus["1|FB"]
        <<switchsStatus["3|DB"]
        <<switchsStatus["3|FB"]
        <<switchsStatus["5|DB"]
        <<switchsStatus["5|FB"]
        <<switchsStatus["7|DB"]
        <<switchsStatus["7|FB"]
       <<switchsStatus["9|DB"]
       <<switchsStatus["9|FB"]
       <<switchsStatus["11|DB"]
       <<switchsStatus["11|FB"]
       <<switchsStatus["2|DB"]
       <<switchsStatus["2|FB"]
       ;


}



