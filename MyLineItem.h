#ifndef MYLINEITEM_H
#define MYLINEITEM_H

#include <QGraphicsLineItem>
#include <QObject>

class MyLineItem:public QObject, public QGraphicsLineItem
{
Q_OBJECT
public:
    MyLineItem(QLineF & line, QObject* parentobj=nullptr, QGraphicsLineItem * parentitem=nullptr);
    MyLineItem(QLineF & line, QColor color, QObject* parentobj=nullptr, QGraphicsLineItem * parentitem=nullptr);

    void setColor(QColor  &color);
    void setLineName(QString lineName);

protected:
   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

//我们可以记线段的位置
private:
   QLineF  line;
   QColor  color;
   QString lineName; //线段名称

};

#endif // MYLINEITEM_H
