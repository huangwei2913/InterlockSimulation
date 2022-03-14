#include "MenuPushButton.h"
#include <QWidget>
#include <QDebug>
#include <QPalette>
MenuPushButton::MenuPushButton(const QString &text, QWidget *parent ):QPushButton(text,parent)
{
    jobdoneflag =0;  //假设该按钮所对应的工作还没有执行
    clearflag = 0;
    setStyleSheet("background-color: rgb(115, 115, 115);");
    connect(this,&MenuPushButton::clicked,this, &MenuPushButton::buttonClicked);
}
MenuPushButton::~MenuPushButton(){

}


void  MenuPushButton::exterClearButtonClicked(){
   clearflag=1;
   setStyleSheet("background-color: rgb(115, 115, 115);");
   clearflag=0;
   jobdoneflag =0;  //假设该按钮所对应的工作还没有执行
}


void MenuPushButton::timeoutjob(){
    if(jobdoneflag==0){   //这么长时间了居然还没有完成任何工作
        the_last_clicked_time = QTime::currentTime();
        setStyleSheet("background-color: rgb(115, 115, 115);");

    }else{
         //虽然工作正在做，但是接收到clear指令
         //这个地方等后面加入命令执行进行
    }
}

void  MenuPushButton::buttonClicked(){
  //改变button背景颜色，到后面会有更多逻辑需要在这里实现
  qDebug()<<"mdclick..........";
  if(this->the_last_clicked_time.isNull()==true){
    the_last_clicked_time = QTime::currentTime();
    setStyleSheet("background-color: rgb(255, 0, 255);");
    //这个地方开始启动一次QTimer事件，这个事件主要处理，在15s后会自动回复原来按钮状态的
    QTimer::singleShot( 15000, this, SLOT(timeoutjob()) );


  }else{   //如果又被点击了，并且在15s以内则返回，不进行任何动作，
    int judgeinterval = QTime::currentTime().secsTo(the_last_clicked_time);  //判断间隔时间
    if(judgeinterval<15){
        if(jobdoneflag==0){         //例如clear被点击会出现这个情况
            the_last_clicked_time = QTime::currentTime();
            setStyleSheet("background-color: rgb(255, 0, 255);");
            //这个地方开始启动一次QTimer事件，这个事件主要处理，在15s后会自动回复原来按钮状态的
            QTimer::singleShot( 15000, this, SLOT(timeoutjob()) );
        }
        else{
            return;
        }

    }else{ //大于15s了，这个时候需要判断任务是否已经完成，如果任务已经完成，又接收到点击事件，则开始进入下一轮同样操作钟，也就说上面的两句都要再执行一次

       if(jobdoneflag==1){  //上一轮工作已经完成，又接收到点击事件
           the_last_clicked_time = QTime::currentTime();
           jobdoneflag=0;
           setStyleSheet("background-color: rgb(255, 0, 255);");
           //又开始启动一次QTimer事件
       } else{

           //如果任务没有完成，则返回
            return;
       }

    }

  }

}

