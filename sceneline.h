#ifndef SCENELINE_H
#define SCENELINE_H

#include <QGraphicsObject>
#include <QGraphicsTextItem>

//线条来模拟轨道区段，进而知道区段在上位机界面中的颜色
class SceneLine : public QGraphicsObject
{
    Q_OBJECT
public:
    SceneLine(const QPointF &start, const QPointF &end,
              const QString &tracksectionName, bool textpostion, QGraphicsItem *parent = nullptr);

    void setColor(const QColor &color);
    QColor color() const;

    void setTextVisible(bool visible);
    void setTextPositionAbove(bool above);
    void setTextColor(const QColor &color);  //代表的是线条所对应的解释文本的颜色
    QString getColor() const        //代表的是线条的颜色
    {
        if (m_color == Qt::red)
            return "Red";
        else if (m_color == Qt::white)
            return "White";
        else
            return "Default";
    }

    void restore()      //恢复至上一次设置的状态
    {
        m_color = m_lastColor;
        update();
    }

    QString getTracksectionName() { return this->m_tracksectionName; }

    QPointF getStart() const { return m_line.p1(); }
    QPointF getEnd() const { return m_line.p2(); }



public slots:
    void changeColor(QString color);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;

private:
    QColor m_color;
    QString m_tracksectionName;
    QGraphicsTextItem *m_textItem;
    QLineF m_line;
    bool textpostion;
    QColor m_lastColor;


};

#endif // SCENELINE_H
