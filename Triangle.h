#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <QObject>
#include <QObject>
#include <QTimer>
#include <QGraphicsObject>

#define GO true
#define STOP false

class Triangle : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit Triangle();

signals:
       void signalmainwindow(int mytestval);

private:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

public slots:
    void slotTarget(QPointF point);

private:
    QTimer *gameTimer;      // Игровой таймер
    QPointF target;         // Положение курсора
    bool state;             // Статус идти/стоять
    int mytestval;

private slots:
    void slotGameTimer();   // Игровой слот

public:
    int getmytestval();

protected:
     bool event(QEvent *ev) override;
};

#endif // TRIANGLE_H
