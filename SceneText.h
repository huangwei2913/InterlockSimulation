#ifndef SCENETEXT_H
#define SCENETEXT_H

#include <QGraphicsObject>
#include <QObject>

//用这个文本来模拟转辙机

#include <QGraphicsObject>
#include <QGraphicsTextItem>

class SceneText : public QGraphicsObject
{
    Q_OBJECT
public:
    SceneText(const QString& switchName, const QPointF& pos, const QColor& color, QGraphicsItem* parent = nullptr);

    void setColor(const QString& color);
    QString getColor() const;
    QString getSwitchName() { return m_switchName; }
    void restore();

public slots:
    void changeColor(QString color);

signals:
    void clicked(const QString& switchName);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    bool eventFilter(QObject* obj, QEvent* event) override; // 添加事件过滤器

private:
    QColor m_color;
    QString m_switchName;
    QGraphicsTextItem* m_textItem;
    QColor lastColor;
};
#endif // SCENETEXT_H
