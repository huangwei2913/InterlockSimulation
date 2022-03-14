#ifndef CPPOBJECT_H
#define CPPOBJECT_H

#include <QObject>
#include <QDebug>

//这个类对象只不过是qml界面元素和内部逻辑的桥梁
class CppObject : public QObject
{
    Q_OBJECT
public:
    explicit CppObject(QObject *parent = nullptr);

signals:
    void dispalyguzhansignal(const QString & link);

public slots:
    //槽函数 --用来接收qml的信号,第一个参数代表第几个故障点，第二个参数表示被点击了多少下
    void cppRecvMsg(const QString &arg1,const QString &arg2);

};

#endif // CPPOBJECT_H
