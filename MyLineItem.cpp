#include "MyLineItem.h"
#include <QPainter>

MyLineItem::MyLineItem(QLineF & line,QObject* parentobj, QGraphicsLineItem * parentitem):QObject(parentobj),QGraphicsLineItem(parentitem)
{
    this->line=line;
    color.setRed(77);
    color.setGreen(118);
    color.setBlue(179);
}

void MyLineItem::setLineName(QString lineName){
    this->lineName =lineName;
}

void MyLineItem::setColor(QColor &color){
    this->color=color;
    this->update();
}

void MyLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

     QPen pen;
     pen.setColor(color);
     pen.setWidth(5);
     pen.setBrush(QBrush(color));
     painter->setPen(pen);
     painter->drawLine(line);
}
