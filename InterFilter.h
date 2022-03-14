#ifndef INTERFILTER_H
#define INTERFILTER_H

//主要用于场景类中做元项的更新操作
#include <QObject>

class InterFilter : public QObject
{
    Q_OBJECT 
public:
    explicit InterFilter(QString message,QObject *parent = nullptr);
protected:
    bool eventFilter( QObject *dest, QEvent *event );
signals:
public slots:
private:
    QString m_message;

};

#endif // INTERFILTER_H
