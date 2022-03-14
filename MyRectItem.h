#ifndef MYRECTITEM_H
#define MYRECTITEM_H

#include <QObject>
#include <QGraphicsRectItem>


//定义的是一个矩形区域，接受外部事件改变颜色，这个到后期统一来做
class MyRectItem:public QObject, public QGraphicsRectItem
{
Q_OBJECT
public:
    MyRectItem(const QRectF &rect,QObject* parentobj=nullptr, QGraphicsItem * parentitem=nullptr);

    QRectF boundingRect() const;

protected:
   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;




 private:
    QRectF rect;







};

#endif // MYRECTITEM_H
