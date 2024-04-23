#include "SceneText.h"

#include <QPen>
#include <QPainter>
#include <QFont>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

SceneText::SceneText(const QString& switchName, const QPointF& pos, const QColor& color, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_switchName(switchName)
{
    m_textItem = new QGraphicsTextItem(m_switchName, this);
    QFont font = m_textItem->font();
    font.setPointSize(10);
    m_textItem->setFont(font);
    m_textItem->setDefaultTextColor(Qt::green);
    m_textItem->setPos(pos);
    m_color = color;
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    // 安装事件过滤器
    m_textItem->installEventFilter(this);
}

void SceneText::restore()
{
    m_color = lastColor;
    m_textItem->setDefaultTextColor(m_color);
    update();
}

void SceneText::setColor(const QString& color)
{
    lastColor = m_color;
    if (color == "Red")
        m_color = Qt::red;
    else if (color == "Green")
        m_color = Qt::green;
    else if (color == "Yellow")
        m_color = Qt::yellow;
    else
        m_color = Qt::yellow;

    m_textItem->setDefaultTextColor(m_color);
    update();
}

QString SceneText::getColor() const
{
    if (m_color == Qt::red)
        return "Red";
    else if (m_color == Qt::green)
        return "Green";
    else
        return "Yellow";
}

void SceneText::changeColor(QString color)
{
    setColor(color);
}

void SceneText::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

QRectF SceneText::boundingRect() const
{
    return m_textItem->boundingRect();
}

bool SceneText::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_textItem && event->type() == QEvent::GraphicsSceneMousePress)
    {
        QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
        // 在这里处理鼠标按下事件
        qDebug() << "Mouse Press Event triggered...";
        emit clicked(m_switchName);
        return true; // 事件已被处理,不需要继续传播
    }
    return QGraphicsObject::eventFilter(obj, event);
}

void SceneText::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // 在这里处理鼠标按下事件
    qDebug() << "Mouse Press Event triggered...";
    emit clicked(m_switchName);
    QGraphicsObject::mousePressEvent(event);
}

void SceneText::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    setOpacity(0.7);
    QGraphicsObject::hoverEnterEvent(event);
}

void SceneText::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    setOpacity(1.0);
    QGraphicsObject::hoverLeaveEvent(event);
}

QPainterPath SceneText::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}