#include "Triangle.h"
#include <math.h>
#include <QPainter>
#include <QEvent>
#include "Interlockingevent.h"
#include <QDebug>

static const double Pi = 3.14159265358979323846264338327950288419717;
static double TwoPi = 2.0 * Pi;

static qreal normalizeAngle(qreal angle)
{
    while (angle < 0)
        angle += TwoPi;
    while (angle > TwoPi)
        angle -= TwoPi;
    return angle;
}

Triangle::Triangle()
    : QGraphicsObject()
{
    setRotation(0);
    state = STOP;
    gameTimer = new QTimer();   // Инициализируем игровой таймер
    // Подключаем сигнал от таймера и слоту обработки игрового таймера
    connect(gameTimer, &QTimer::timeout, this, &Triangle::slotGameTimer);
    gameTimer->start(1);   // Стартуем таймер
    mytestval = 0;
}

QRectF Triangle::boundingRect() const
{
    return QRectF(-12,-15,24,30);
}

void Triangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Отрисовка треугольника
    QPolygon polygon;
    polygon << QPoint(0,-15) << QPoint(12,15) << QPoint(-12,15);
    painter->setBrush(Qt::red);
    painter->drawPolygon(polygon);

    Q_UNUSED(option);
    Q_UNUSED(widget);
}

void Triangle::slotGameTimer()
{
    if(state){
        QLineF lineToTarget(QPoint(0,0), mapFromScene(target));
        if(lineToTarget.length() > 2){
             setPos(mapToParent(0, -2));
        }
        /* Проверка выхода за границы поля
         * Если объект выходит за заданные границы, то возвращаем его назад
         * */
        if(this->x() - 30 < 0){
            this->setX(30);         /// слева
            state = STOP;           // Останавливаемся
        }
        if(this->x() + 30 > 520){
            this->setX(520 - 30);   /// справа
            state = STOP;           // Останавливаемся
        }

        if(this->y() - 30 < 0){
            this->setY(30);         /// сверху
            state = STOP;           // Останавливаемся
        }
        if(this->y() + 30 > 520){
            this->setY(520 - 30);   /// снизу
            state = STOP;           // Останавливаемся
        }
    }
}

void Triangle::slotTarget(QPointF point)
{
    // Определяем расстояние до цели
    target = point;
    QLineF lineToTarget(QPointF(0, 0), mapFromScene(target));
    // Угол поворота в направлении к цели
    qreal angleToTarget = ::acos(lineToTarget.dx() / lineToTarget.length());
    if (lineToTarget.dy() < 0)
        angleToTarget = TwoPi - angleToTarget;
    angleToTarget = normalizeAngle((Pi - angleToTarget) + Pi / 2);

    // Поворачиваем героя к цели
    if (angleToTarget >= 0 && angleToTarget < Pi) {
        // Rotate left
        setRotation(rotation() - angleToTarget * 180 /Pi);
    } else if (angleToTarget <= TwoPi && angleToTarget > Pi) {
        // Rotate right
        setRotation(rotation() + (angleToTarget - TwoPi )* (-180) /Pi);
    }

    state = GO; // Разрешаем идти
}

bool Triangle::event(QEvent *ev){
  QEvent::Type eventType = ev->type();

  if((eventType == Interlockingevent::eventType))
   {
      /*在这里处理事件*/
       Interlockingevent *myEvent = dynamic_cast<Interlockingevent*>(ev);
       qDebug() << myEvent->getValue1();
       qDebug() << myEvent->getValue2();
       qDebug()<<"I have gotten the custome event in the Triangle ............. ";
       //我们将该事件发送到graphicitem
       //改变Triangle的颜色
        mytestval = 1000;   //接着我们会将信号与槽链接起来，让主窗口明白其值已经更新了
        emit signalmainwindow(mytestval);
        return true;  //表示该自定义事件已经被处理
  }

  //qDebug() << "all Events go here before dispathed....";
  return QGraphicsObject::event(ev);

}


int Triangle::getmytestval(){return mytestval;}
