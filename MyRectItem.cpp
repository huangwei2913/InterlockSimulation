#include "MyRectItem.h"
#include <QPainter>
#include <QRectF>

MyRectItem::MyRectItem(const QRectF &rect,QObject* parentobj, QGraphicsItem * parentitem):QObject(parentobj),QGraphicsRectItem(parentitem)
{
this->rect = rect;
}


void MyRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

     painter->setBrush(QBrush(Qt::red));
     painter->drawRect(rect);

}


QRectF  MyRectItem::boundingRect() const{
 return this->rect;
}
