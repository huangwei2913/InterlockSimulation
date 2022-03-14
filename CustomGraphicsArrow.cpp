#include "CustomGraphicsArrow.h"
#include <QPainter>
#include <QPolygon>
CustomGraphicsArrow::CustomGraphicsArrow(QGraphicsItem * parent):QGraphicsLineItem(parent)
{
}


void CustomGraphicsArrow::setSourceAndDestPoint(QPointF &source, QPointF &dest){
   this->source =source;
   this->dest  = dest;
   this->line  =QLineF(this->source,  this->dest);
   this->line.setLength(this->line.length()-20);
}



void  CustomGraphicsArrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

   QGraphicsLineItem::paint(painter,option,widget);
   painter->save();
   QPen pen;
   pen.setJoinStyle(Qt::MiterJoin);
   pen.setWidth(2);
   pen.setColor(Qt::white);
   painter->setPen(pen);
   QBrush brush;
   brush.setColor(Qt::white);
   brush.setStyle(Qt::SolidPattern);
   painter->setBrush(brush);
   QLineF v = line.unitVector();
   v.setLength(20);
   v.translate(QPointF(line.dx(), line.dy()));
   auto n = v.normalVector();
   n.setLength(n.length() * 0.5);
   auto n2 = n.normalVector().normalVector();
   auto p1 = v.p2();
   auto p2 = n.p2();
   auto p3 = n2.p2();
   QPolygonF polygon;
   polygon.append(p1);
   polygon.append(p2);
   polygon.append(p3);
   painter->drawLine(line);
   painter->drawPolygon(polygon);
   painter->restore();
}
