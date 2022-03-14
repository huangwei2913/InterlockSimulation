#include "InterFilter.h"
#include <QDebug>

InterFilter::InterFilter(QString message,QObject *parent) : QObject(parent),
    m_message(message)
{

}


bool InterFilter::eventFilter(QObject *dest, QEvent *event)
{
    qDebug() << "Event filter entering.................. " << m_message;
    return QObject::eventFilter(dest,event);
}
