#include "Interlockingevent.h"

//互锁的事件类型

QEvent::Type Interlockingevent::eventType = (QEvent::Type)QEvent::registerEventType(QEvent::User+1);


Interlockingevent::Interlockingevent(int value1, double value2)
            : QEvent(Type(eventType)), _value1(value1), _value2(value2)
{

}

int Interlockingevent::getValue1() {
    return _value1;
}
double Interlockingevent::getValue2() {
    return _value2;
}
