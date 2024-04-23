#include "SceneEclipse.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QDebug>
SceneEclipse::SceneEclipse(qreal x, qreal y, QGraphicsScene *parent)
    : QGraphicsObject(), rect(x, y, 20, 20), desiredColor(Qt::red), parent(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
}


SceneEclipse::SceneEclipse(qreal x, qreal y, QGraphicsScene* parent, const QString& initialColor)
    : QGraphicsObject(), rect(x, y, 20, 20), parent(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);

    // 根据 initialColor 设置 desiredColor
    if (initialColor == "Red")
        desiredColor = Qt::red;
    else if (initialColor == "Green")
        desiredColor = Qt::green;
    else if (initialColor == "Yellow")
        desiredColor = Qt::yellow;
    else if (initialColor == "White")
        desiredColor = Qt::white;
    else if (initialColor == "Black")
        desiredColor = Qt::black;
    else
        desiredColor = Qt::red; // 如果 initialColor 无效，则默认设置为红色
}


QRectF SceneEclipse::boundingRect() const
{
    return rect;
}

void SceneEclipse::setSignalName(QString signalName)
{
    this->signalName = signalName;
}

QString SceneEclipse::getColorState() const
{
    if (desiredColor == Qt::red)
        return "Red";
    else if (desiredColor == Qt::green)
        return "Green";
    else if (desiredColor == Qt::yellow)
        return "Yellow";
    else if (desiredColor == Qt::white)
        return "White";
    else if (desiredColor == Qt::black)
        return "Black";
    else
        return "Unknown";
}

void SceneEclipse::setColorState(QString color)
{
    qDebug() << "color going to be..." << color;
    lastColor = desiredColor;
    if (color == "Red")
    {
        desiredColor = Qt::red;
    }
    if (color == "Green")
    {
        desiredColor = Qt::green;
    }

    if (color == "Yellow")
    {
        desiredColor = Qt::yellow;
    }
    if (color == "White")
    {
        desiredColor = Qt::white;
    }
    if (color == "Black")
    {
        desiredColor = Qt::black;
    }
    update(this->boundingRect());
}

void SceneEclipse::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
       Q_UNUSED(widget);
    painter->setBrush(desiredColor);
    QRectF circleRect(rect.x(), rect.y(), rect.width(), rect.width());
     painter->drawEllipse(circleRect);
}


void SceneEclipse::restore()
{
    // 将 desiredColor 恢复到 lastColor
    desiredColor = lastColor;

    update();
}