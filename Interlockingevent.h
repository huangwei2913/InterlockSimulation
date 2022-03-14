#ifndef INTERLOCKINGEVENT_H
#define INTERLOCKINGEVENT_H

#include <QObject>
#include <QEvent>

class Interlockingevent : public QEvent
{
public:
    Interlockingevent(int value1, double value2);
    static Type eventType;

    int getValue1();
    double getValue2();


private:
   int _value1;   //火车位置
   double _value2;

};

#endif // INTERLOCKINGEVENT_H
