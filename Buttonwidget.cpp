#include "Buttonwidget.h"
#include <QDebug>
#include <QInputDialog>
#include <QDir>

ButtonWidget::ButtonWidget(QPushButton *parent) : QPushButton(parent)
{
    setMouseTracking(true);
    this->enable_falsh =0;  //0表示不启动闪烁
    flagcolor=false;
    startTimer(1000);
}

void ButtonWidget::timerEvent(QTimerEvent *event){
//  qDebug()<<"the button widget starting... enable_falsh="<<enable_falsh<<this->name;

//  if(enable_falsh==1){
//      qDebug()<<"MBBBBBBBBBBBBBBBBB.........="<<this->name;
//  }

  if(this->name=="SI-A"){

      //qDebug()<<"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.........="<<enable_falsh;

      if(enable_falsh==0){
        //我感觉自己明白了，要控制这里
        setStyleSheet("background-color: rgb(0,255,0)");
      }else{  //启动闪烁
       flagcolor=!flagcolor;
       if(flagcolor){
          setStyleSheet("background-color: rgb(0,255,0)");
       }
       else{
           setStyleSheet("background-color: rgb(0,0,0)");
       }
      }
      return;
  }

  if(this->name=="X-A1"){
      if(enable_falsh==0){
        //我感觉自己明白了，要控制这里
        setStyleSheet("background-color: rgb(0,255,0)");

      }else{  //启动闪烁
       flagcolor=!flagcolor;

       if(flagcolor){
          setStyleSheet("background-color: rgb(0,255,0)");
       }
       else{
           setStyleSheet("background-color: rgb(0,0,0)");
       }
      }
      return;
  }



  if(this->name=="S3-A"){
      if(enable_falsh==0){
        //我感觉自己明白了，要控制这里
        setStyleSheet("background-color: rgb(0,255,0)");

      }else{  //启动闪烁
       flagcolor=!flagcolor;

       if(flagcolor){
          setStyleSheet("background-color: rgb(0,255,0)");
       }
       else{
           setStyleSheet("background-color: rgb(0,0,0)");
       }
      }
      return;
  }



  if(this->name=="SII-A"){
      if(enable_falsh==0){
        //我感觉自己明白了，要控制这里
        setStyleSheet("background-color: rgb(0,255,0)");

      }else{  //启动闪烁
       flagcolor=!flagcolor;

       if(flagcolor){
          setStyleSheet("background-color: rgb(0,255,0)");
       }
       else{
           setStyleSheet("background-color: rgb(0,0,0)");
       }
      }
      return;
  }


  if(this->name=="S4-A"){
      if(enable_falsh==0){
        //我感觉自己明白了，要控制这里
        setStyleSheet("background-color: rgb(0,255,0)");

      }else{  //启动闪烁
       flagcolor=!flagcolor;

       if(flagcolor){
          setStyleSheet("background-color: rgb(0,255,0)");
       }
       else{
           setStyleSheet("background-color: rgb(0,0,0)");
       }
      }
      return;
  }



  if(this->name=="XN-A1"){
      if(enable_falsh==0){
        //我感觉自己明白了，要控制这里
        setStyleSheet("background-color: rgb(0,255,0)");

      }else{  //启动闪烁
       flagcolor=!flagcolor;

       if(flagcolor){
          setStyleSheet("background-color: rgb(0,255,0)");
       }
       else{
           setStyleSheet("background-color: rgb(0,0,0)");
       }
      }
      return;
  }



  if(this->name=="X4-A"){
      if(enable_falsh==0){
        //我感觉自己明白了，要控制这里
        setStyleSheet("background-color: rgb(0,255,0)");

      }else{  //启动闪烁
       flagcolor=!flagcolor;

       if(flagcolor){
          setStyleSheet("background-color: rgb(0,255,0)");
       }
       else{
           setStyleSheet("background-color: rgb(0,0,0)");
       }
      }
      return;
  }


  if(this->name=="SN-A1"){
      if(enable_falsh==0){
        //我感觉自己明白了，要控制这里
        setStyleSheet("background-color: rgb(0,255,0)");

      }else{  //启动闪烁
       flagcolor=!flagcolor;

       if(flagcolor){
          setStyleSheet("background-color: rgb(0,255,0)");
       }
       else{
           setStyleSheet("background-color: rgb(0,0,0)");
       }
      }
      return;
  }



  if(this->name=="S-A1"){
      if(enable_falsh==0){
        //我感觉自己明白了，要控制这里
        setStyleSheet("background-color: rgb(0,255,0)");

      }else{  //启动闪烁
       flagcolor=!flagcolor;

       if(flagcolor){
          setStyleSheet("background-color: rgb(0,255,0)");
       }
       else{
           setStyleSheet("background-color: rgb(0,0,0)");
       }
      }
      return;
  }


  if(this->name=="XII-A"){
      if(enable_falsh==0){
        //我感觉自己明白了，要控制这里
        setStyleSheet("background-color: rgb(0,255,0)");

      }else{  //启动闪烁
       flagcolor=!flagcolor;

       if(flagcolor){
          setStyleSheet("background-color: rgb(0,255,0)");
       }
       else{
           setStyleSheet("background-color: rgb(0,0,0)");
       }
      }
      return;
  }


}


void ButtonWidget::changeFlash(int enable_falsh){
    enable_falsh = enable_falsh;
}

void ButtonWidget::setButtonName(QString &name){
    this->name =name;
}


//取消选择某个按钮
void ButtonWidget::slotcacelbuttonselect(QString item_name){
    if(this->name==item_name){  //在这里实现逻辑控制
            //在这个地方安排进路
            this->enable_falsh=0;
            this->update();
            //在这里加入处理逻辑，如果超过十五秒还没有
            //在这里进行处理
            //在这里将
            emit  routecacleRequestMessage(item_name);   //取消进路请求
    }
}

void ButtonWidget::slotButtonTarget(QString item_name){

     //引导进路要特殊处理一下
    if(this->name==item_name){  //在这里实现逻辑控制

        if("X-A1"==item_name){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            emit  routeRequestMessage(item_name);
            this->enable_falsh=1;
            this->update();
            //在这里加入处理逻辑，如果超过十五秒还没有
            return;
        }

        if("S-A1"==item_name){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            emit  routeRequestMessage(item_name);
            this->enable_falsh=1;
            this->update();
            //在这里加入处理逻辑，如果超过十五秒还没有
            return;
        }


        if("XII-A"==item_name){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            emit  routeRequestMessage(item_name);
            this->enable_falsh=1;
            this->update();
            return;
        }


        if("SI-A"==item_name){
            emit  routeRequestMessage(item_name);
            this->enable_falsh=1;
            this->update();
            return;
        }
        if("S3-A"==item_name){
            emit routeRequestMessage(item_name);
            this->enable_falsh=1;
            this->update();
            return;

        }
        if("SII-A"==item_name){
            emit  routeRequestMessage(item_name);
            this->enable_falsh=1;
            this->update();
            return;
        }

        if("S4-A"==item_name){
            emit  routeRequestMessage(item_name);
            this->enable_falsh=1;
            this->update();
            return;
        }
        if("X4-A"==item_name){
            emit  routeRequestMessage(item_name);
            this->enable_falsh=1;
            this->update();
            return;
        }

        if("SN-A1"==item_name){
            emit  routeRequestMessage(item_name);
            this->enable_falsh=1;
            this->update();
            return;
        }


        if("XN-A1"==item_name){
            emit  routeRequestMessage(item_name);
            this->enable_falsh=1;
            this->update();
            return;
        }

        if("X-A2"==item_name){
            emit popUpDialogMessage();          //弹出对话框
            this->update();
            return;
        }

    }

}

void ButtonWidget::slotBUttonStopFlash(QString item_name){

//双线操作的时候里面和外面的都是需要的
    if(this->name==item_name){

        if("X-A1"==item_name){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            this->enable_falsh=0;
            this->update();
            return;
        }

        if(item_name=="SI-A"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }

        if(item_name=="SII-A"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }

        if(item_name=="S4-A"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }


        if(item_name=="S3-A"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }

        if(item_name=="XN-A1"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }

        if(item_name=="X4-A"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }


        if(item_name=="SN-A1"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }



        if(item_name=="S-A1"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }

        if(item_name=="XII-A"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }


    }

    if(this->name==item_name){   //

        if(item_name=="SI-A"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }
        if(item_name=="X-A1"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            this->enable_falsh=0;
            this->update();
            return;
        }

        if(item_name=="SII-A"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }


        if(item_name=="S4-A"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }

        if(item_name=="S3-A"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }

        if(item_name=="X4-A"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }



        if(item_name=="XN-A1"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }


        if(item_name=="SN-A1"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }



        if(item_name=="S-A1"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }

        if(item_name=="XII-A"){   //针对不同的按钮做不同的处理，所有按钮都不能移动
            //在这个地方安排进路
            enable_falsh=0;
            this->update();
            return;
        }




    }


}

