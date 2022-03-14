#include "Myellipseitem.h"
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPen>
#include <QPainter>

MyEllipseItem::MyEllipseItem(const QRectF &rect, QGraphicsItem *parent)
    : QGraphicsEllipseItem(rect, parent)
{
    this->isHovered = false;
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    this->rect = rect;
}

MyEllipseItem::MyEllipseItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent)
    : QGraphicsEllipseItem(x, y, w, h, parent)
{
    this->isHovered = false;
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    this->rect.setX(x);
    this->rect.setY(y);
    this->rect.setWidth(w);
    this->rect.setHeight(h);
}

QRectF MyEllipseItem::boundingRect() const
{
    return this->rect;
}

void MyEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    if (this->isSelected())
    {
        pen.setColor(Qt::red);
    }
    else
    {
        pen.setColor(Qt::green);
    }
    if (this->isHovered)
    {
        pen.setWidth(4);
    }
    else
    {
        pen.setWidth(2);
    }
    painter->setPen(pen);
    painter->drawEllipse(this->rect);
}

QPainterPath MyEllipseItem::shape() const
{
    QPainterPath temp;
    temp.addEllipse(this->boundingRect());
    QPainterPathStroker pathStroker;

    QPainterPath path = pathStroker.createStroke(temp);
    return path;
}

void MyEllipseItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    this->isHovered = true;
    prepareGeometryChange();
}

void MyEllipseItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    this->isHovered = false;
    prepareGeometryChange();



}
