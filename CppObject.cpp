#include "CppObject.h"

CppObject::CppObject(QObject *parent) : QObject(parent)
{

}


void CppObject::cppRecvMsg(const QString &arg1,const QString &arg2){
    //将这些信息存储到列表中
    int clincked_num = arg2.toInt();
    QString temp="故障: ";
    if(clincked_num%2==0){
     temp  = temp+arg1 +" 已取消";
    }else{
     temp  = temp+arg1 +" 已设置";
    }
    emit dispalyguzhansignal(temp);

}
