#include "sceneline.h"
#include <QPen>
#include <QPainter>

SceneLine::SceneLine(const QPointF &start, const QPointF &end,
                     const QString &tracksectionName,bool textpos, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_tracksectionName(tracksectionName), m_line(start, end), textpostion(textpos)
{
    m_textItem = new QGraphicsTextItem(m_tracksectionName, this);
    QFont font = m_textItem->font();
        font.setPointSize(10);  // 设置字体大小为20
        m_textItem->setFont(font);
    m_textItem->setDefaultTextColor(Qt::yellow);
    m_textItem->setPos((start + end) / 2);

    // 设置默认颜色
    m_color.setRed(77);
    m_color.setGreen(118);
    m_color.setBlue(179);
}

void SceneLine::setColor(const QColor &color)
{
    m_lastColor = m_color;
    m_color = color;
    update();
}

QColor SceneLine::color() const
{
    return m_color;
}

void SceneLine::setTextVisible(bool visible)
{
    m_textItem->setVisible(visible);
}

void SceneLine::setTextPositionAbove(bool above)
{
    if (above) {
        QPointF textPos = (m_line.p1() + m_line.p2()) / 2;
        textPos.setX(textPos.x() - m_textItem->boundingRect().width() / 2);
        textPos.setY(textPos.y() - m_textItem->boundingRect().height() / 2 - 20);  // 文本上移20个单位
        m_textItem->setPos(textPos);
    }
    else {
    
        QPointF textPos = (m_line.p1() + m_line.p2()) / 2;
        textPos.setX(textPos.x() - m_textItem->boundingRect().width() / 2);
        textPos.setY(textPos.y() - m_textItem->boundingRect().height() / 2 +20);  // 文本下移20个单位
        m_textItem->setPos(textPos);
    
    }

}

void SceneLine::setTextColor(const QColor &color)
{
    m_textItem->setDefaultTextColor(color);
    update();
}

void SceneLine::changeColor(QString color)
{
    if (color == "Red")
        setColor(Qt::red);
    else if (color == "White")
        setColor(Qt::white);
    else
        setColor(m_color);
}

void SceneLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

      Q_UNUSED(option);
      Q_UNUSED(widget);

      QPen pen(m_color);
      pen.setWidth(10);  // 设置线段宽度
      painter->setPen(pen);
      painter->drawLine(m_line);

      // 设置文本的位置
      // 设置文本的位置
      if (textpostion == true) {
          QPointF textPos = (m_line.p1() + m_line.p2()) / 2;
          textPos.setX(textPos.x() - m_textItem->boundingRect().width() / 2);
          textPos.setY(textPos.y() - m_textItem->boundingRect().height() / 2 - 20);  // 文本上移10个单位
          m_textItem->setPos(textPos);
      }
      else {
          QPointF textPos = (m_line.p1() + m_line.p2()) / 2;
          textPos.setX(textPos.x() - m_textItem->boundingRect().width() / 2);
          textPos.setY(textPos.y() - m_textItem->boundingRect().height() / 2 + 20);  // 文本上移10个单位
          m_textItem->setPos(textPos);
      }
     


}

QRectF SceneLine::boundingRect() const
{
    qreal penWidth = 10;  // 与上面设置的线段宽度一致
    qreal extra = (penWidth + 2) / 2.0;

    return QRectF(m_line.p1(), QSizeF(m_line.p2().x() - m_line.p1().x(), m_line.p2().y() - m_line.p1().y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);


}
