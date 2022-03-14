#ifndef MYSIGNAL_H
#define MYSIGNAL_H

#include <QObject>
#include <QGraphicsObject>

class   Mysignal: public QGraphicsObject
{
Q_OBJECT
public:
    Mysignal(QRectF & rect,QGraphicsScene *parent, QColor &thecolor);
    Mysignal(qreal x, qreal y, qreal w, qreal h,QGraphicsScene *parent, QColor &thecolor);
    QRectF boundingRect() const override;
    void setSignalId(int id); //设置信号机ID
    void setSignalName(QString signalName);
    void setDesignedColor(QColor desired_colar);

public slots:
       void slotsingalColorSpecfied(QString item_name,QString color);  //此槽函数接收从topscene发出的信号变化指令执行指定操作

protected:
   void timerEvent(QTimerEvent *) override; //
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QRectF rect;
    bool enable_flash;     //开始闪烁么
    QColor desiredcolar;   //想要的颜色
    int flash_interval;    //默认闪烁的时间间隔
    QGraphicsScene *parent; //用于通知更新
    int singleid;           //这个线废弃不用
    bool noflash;
    qreal pointx;
    qreal pointy;
    QString signalName;         //信号灯名称


};

#endif // MYSIGNAL_H
