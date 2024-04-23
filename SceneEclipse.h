#ifndef SCENEECLIPSE_H
#define SCENEECLIPSE_H

#include <QGraphicsObject>
#include <QObject>

//这个类用来模拟界面中要显示信号机
class SceneEclipse : public QGraphicsObject
{
    Q_OBJECT

public:
    SceneEclipse(qreal x, qreal y, QGraphicsScene *parent);
    SceneEclipse(qreal x, qreal y, QGraphicsScene* parent, const QString& initialColor);
    QRectF boundingRect() const override;
    void setSignalName(QString signalName);
    QString getSignalName() { return signalName; }
    QString getColorState() const;

    void restore(); //恢复到上次色灯信号状态


public slots:
    void setColorState(QString color);

private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QRectF rect;
    QColor desiredColor;
    QGraphicsScene *parent;
    QString signalName;
    QString colorState;
    QColor lastColor;  // 增加一个 QColor 类型的成员变量，用于存储最近一次的颜色

};

#endif // SCENEECLIPSE_H
