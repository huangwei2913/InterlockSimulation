#include "Mysignal.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QDebug>


Mysignal::Mysignal(QRectF &rect,QGraphicsScene *parent,QColor &thecolor):QGraphicsObject(),flash_interval(1000)
{
    this->rect = rect;
    setFlags( QGraphicsItem::ItemIsSelectable |QGraphicsItem::ItemIsMovable |QGraphicsItem::ItemSendsGeometryChanges );
    enable_flash = true;  //默认情况下不闪烁
    startTimer(flash_interval); //开始内部计数器
    this->parent =parent;
    desiredcolar = thecolor;
    noflash = true;  //默认创建的信号灯不闪烁
    pointx = rect.x();
    pointy = rect.y();

}

void Mysignal::slotsingalColorSpecfied(QString item_name,QString color){

    if(this->signalName == item_name){
         //信号灯点什么灯
       if(color=="red"){
           this->desiredcolar =Qt::red;
       }else if (color=="green"){
           this->desiredcolar =Qt::green;
       }else if (color=="yellow"){
           this->desiredcolar =Qt::yellow;
       }
       else if (color=="white"){
          this->desiredcolar =Qt::white;
       }
       else if (color=="black"){
          this->desiredcolar =Qt::black;
       }else {
        return;
       }
      this->update();  //重新刷新
    }


}



Mysignal::Mysignal(qreal x, qreal y, qreal w, qreal h,QGraphicsScene *parent, QColor &thecolor):QGraphicsObject()
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    this->rect.setX(x);
    this->rect.setY(y);
    this->rect.setWidth(w);
    this->rect.setHeight(h);
    enable_flash = true;  //默认情况下不闪烁
    flash_interval=1000;
    startTimer(flash_interval); //开始内部计数器
    this->parent =parent;
    desiredcolar = thecolor;
    noflash = true;  //默认创建的信号灯不闪烁
    pointx = x;
    pointy = y;
}

void Mysignal::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

    if(signalName.contains("1101")==true ||signalName.contains("1103")==true || signalName.contains("1105")==true || signalName.contains("1107")==true
      || signalName.contains("1102")==true ||signalName.contains("1104")==true || signalName.contains("1106")==true || signalName.contains("1108")==true
            ){
        if(!noflash){
        QPen pen;
        pen.setColor(Qt::black);
        painter->setPen(pen);
        painter->setBrush(enable_flash?(desiredcolar):(Qt::black));
        painter->drawEllipse(this->rect);
        }
        else{
            QPen pen;
            pen.setColor(Qt::black);
            painter->setPen(pen);
            painter->setBrush(desiredcolar);
            painter->drawEllipse(this->rect);
        }
        Q_UNUSED(option);
        Q_UNUSED(widget);
        return;
    }
    if(!noflash){
    QPen pen;
    pen.setColor(Qt::white);
    painter->setPen(pen);
    painter->setBrush(enable_flash?(desiredcolar):(Qt::black));
    painter->drawEllipse(this->rect);
    }
    else{
        QPen pen;
        pen.setColor(Qt::white);
        painter->setPen(pen);
        painter->setBrush(desiredcolar);
        painter->drawEllipse(this->rect);
    }
    Q_UNUSED(option);
    Q_UNUSED(widget);

}

QRectF Mysignal::boundingRect() const
{
    return this->rect;
}

//车载设备与地面设备
//调度指挥网络，信号集中监测网，信号安全数据网
//地面设计为基础，（生成行车许可，线路条件），通过车地通信，车载设备计算生成防护曲线
//轨道占用检查，计算行车许可，具有车地通信功能，地--->车单向，
//列控系统是双向的，地面设备有实时流处理能力么？？？
//曲线计算、测速定位、轨道电路故障、
//轨道 线路数据，在车上是固定的
//速度防护 （速度码， 闭塞分区限制速度，闭塞分区目标速度） 增加一个安全保护车段
//分段曲线控制， 地面设备向车载设备一次性发送，车载设备到目标点的一次性目标曲线
//站内闭塞，固定自动闭塞
//准移动闭塞，轨道电路划分固定闭塞分区，由于车载设备和地面设备配合，
//不设置闭塞分区， 设置虚拟信号机，一个闭塞分区只能由一个车占用， 虚拟分区没有， 地面设备自己玩， 虚拟闭塞 只是用在青藏线
//列车定位的准确性，安全性，可靠性，用卫星来寻找列车，青藏线。卫星能寻找到列车么？
//前车尾部，移动闭塞
//应答器列车定位，


void Mysignal::timerEvent(QTimerEvent *)
{

   enable_flash=!enable_flash;
   this->update(); //估计重新执行paint这个函数
}


QVariant Mysignal::itemChange(GraphicsItemChange change, const QVariant &value){
    if (change == ItemPositionChange && scene()){
        QPointF newPos = value.toPointF();
        qDebug()<<"......................new position"<<newPos;
        qDebug()<<"......................orginal position"<<QPointF(pointx,pointy);
        this->update();
        return mapFromScene(QPointF(pointx,pointy));
    }
    return QGraphicsItem::itemChange(change, value);
}


void Mysignal::setSignalId(int id){
    this->singleid =id;
}

void Mysignal::setSignalName(QString temp){
  this->signalName = temp;
}



void Mysignal::setDesignedColor(QColor desired_colar){
    this->desiredcolar =desired_colar;
    this->update();
}
