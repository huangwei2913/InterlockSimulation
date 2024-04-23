#ifndef BLINKINGSQUAREITEM_H
#define BLINKINGSQUAREITEM_H

#include <QObject>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QStateMachine>
#include <QState>
#include <QFinalState>
#include <QSignalTransition>
#include <QBrush>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QThread>
#include <QDateTime>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

//这个类用于模拟正方形按钮的
class BlinkingSquare : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
        Q_PROPERTY(QBrush brush READ brush WRITE setBrush)

public:
    BlinkingSquare(const QString& squareName, const QPointF& pos, const QColor& color, qreal size, QGraphicsItem* parent = nullptr)
        : buttonName(squareName), QObject(), QGraphicsRectItem(parent), timer(new QTimer(this)), clicknum(0), lastClickTime(QDateTime::currentDateTime())
    {
        setRect(0, 0, size, size);
        setBrush(QBrush(color));
        setPos(pos);
        this->m_color = color;
        setupStateMachine();
        connect(timer, &QTimer::timeout, this, &BlinkingSquare::changeColor);
        m_mutex = new QMutex();
    }

    ~BlinkingSquare() {
        delete m_mutex;
    }

    const QString& getbuttonName() const {
        QMutexLocker locker(m_mutex);
        return buttonName;
    }

    void setColor(const QColor& color)
    {
        QMutexLocker locker(m_mutex);
        m_color = color;
        setBrush(QBrush(m_color));
        update();
    }

    QColor getColor() const
    {
        QMutexLocker locker(m_mutex);
        return m_color;
    }

    void stopBliking() {
        //lastClickTime = QDateTime::currentDateTime().addSecs(-11);
    }


    void lockButtonName() {
        m_mutex->lock();
    }

    void unlockButtonName() {
        m_mutex->unlock();
    }



signals:
    void clicked(QString buttonname);

private slots:
    void changeColor()
    {
        if (clicknum % 2 == 0) {
            timer->stop();
            setBrush(QBrush(m_color));
        }
        else {
            if (brush() == QBrush(m_color))
                setBrush(QBrush(Qt::black));
            else
                setBrush(QBrush(m_color));
        }

        if (clicknum % 2 != 0 && lastClickTime.secsTo(QDateTime::currentDateTime()) > 10) {
            setBrush(QBrush(m_color));
            timer->stop();
            {
                QMutexLocker locker(m_mutex);
                qDebug() << "Button name:" << buttonName;
            }
        }
    }

private:
    QTimer* timer;
    QStateMachine machine;
    QState* normalState;
    QState* blinkingState;
    int clicknum = 0;
    QDateTime lastClickTime;
    QColor m_color; //画刷颜色
    const QString buttonName;
    QMutex* m_mutex;  // 用于保护buttonName的互斥锁

    void setupStateMachine()
    {
        normalState = new QState();
        blinkingState = new QState();

        normalState->assignProperty(this, "brush", QBrush(m_color));
        blinkingState->assignProperty(this, "brush", QBrush(Qt::black));

        normalState->addTransition(this, &BlinkingSquare::clicked, blinkingState);
        blinkingState->addTransition(this, &BlinkingSquare::clicked, normalState);

        machine.addState(normalState);
        machine.addState(blinkingState);

        machine.setInitialState(normalState);
        machine.start();
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            clicknum++;
            {
                QMutexLocker locker(m_mutex);
                qDebug() << "which button is clicked......" << buttonName;
            }
            lastClickTime = QDateTime::currentDateTime();
            emit clicked(getbuttonName());
            if (clicknum % 2 != 0) {
                timer->start(1000);
            }
            else {
                timer->stop();
            }
        }
        QGraphicsRectItem::mousePressEvent(event);
    }
};

#endif // BLINKINGSQUAREITEM_H