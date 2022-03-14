#ifndef CUSTOMGRAPHICSARROW_H
#define CUSTOMGRAPHICSARROW_H


//在该类中实现箭头
#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsLineItem>


class CustomGraphicsArrow: public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    CustomGraphicsArrow(QGraphicsItem * parent = 0);

    void setSourceAndDestPoint(QPointF &source, QPointF &dest);


protected:
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) override;

private:
    QPointF source;
    QPointF dest;
    QLineF  line;

};

#endif // CUSTOMGRAPHICSARROW_H
