#ifndef CUSTOMITEMGROUP_H
#define CUSTOMITEMGROUP_H

#include <QObject>
#include <QGraphicsItemGroup>

class CustomItemGroup:public QGraphicsItemGroup
{


public:
    explicit CustomItemGroup(QGraphicsItem *parent = nullptr);


    //我们希望通过重写paint函数实现边框的颜色的宽度调节
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;


};

#endif // CUSTOMITEMGROUP_H
