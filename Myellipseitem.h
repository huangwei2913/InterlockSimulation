#ifndef MYELLIPSEITEM_H
#define MYELLIPSEITEM_H

#include <QObject>
#include <QGraphicsEllipseItem>

class MyEllipseItem: public QObject, public QGraphicsEllipseItem
{
Q_OBJECT
public:
    MyEllipseItem(const QRectF &rect, QGraphicsItem *parent = 0);
    MyEllipseItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent = 0);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPainterPath shape() const;

signals:

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);



private:
    bool isHovered;
    QRectF rect;



};

#endif // MYELLIPSEITEM_H
