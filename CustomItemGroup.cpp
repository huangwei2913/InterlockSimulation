#include "CustomItemGroup.h"
#include <QPen>
#include <QPainter>
#include <QDebug>
CustomItemGroup::CustomItemGroup(QGraphicsItem *parent):QGraphicsItemGroup(parent)
{

}

void CustomItemGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

    // 边框区域颜色
    QColor color = QColor(Qt::white);
    //painter->setPen(QPen(Qt::white, 20));
    // 绘制虚线
    painter->setPen(QPen(color, 3, Qt::SolidLine));
    painter->setBrush(Qt::NoBrush);
    //注意这里的adjusted函数第一个参数表示增加框的左边宽度（负数越小，表示调整的宽度越大），第四个参数表示调正框的高度（正数越小，表示调整的高度越小）
    painter->drawRect(this->childrenBoundingRect().adjusted(-60,-10,20,10));
    //    qDebug()<<"..............."<<this->childrenBoundingRect().height();
    //    qDebug()<<"..............."<<this->childrenBoundingRect().width();
    //    qDebug()<<"............... boundingRect....width"<<this->boundingRect().width();
    //    qDebug()<<"...............boundingRect....height"<<this->boundingRect().height();
    //    qDebug()<<"............... boundingRect....center"<<this->boundingRect().center();
    //    qDebug()<<"...............boundingRect....topLeft"<<this->boundingRect().topLeft();
    //    qDebug()<<"...............boundingRect....bottomRight"<<this->boundingRect().bottomRight();
    QGraphicsItemGroup::paint(painter,option, widget);
}
