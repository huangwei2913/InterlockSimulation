#ifndef BUTTONWIDGET_H
#define BUTTONWIDGET_H

#include <QPushButton>

//在这个类中主要处理按钮点击事件
class ButtonWidget : public QPushButton
{
    Q_OBJECT
public:
    explicit ButtonWidget(QPushButton *parent = nullptr);

signals:
    void routeRequestMessage(QString item_name);  //哪一个按钮正在请求进路
    void routecacleRequestMessage(QString item_name);
    void popUpDialogMessage();

public slots:
    void slotButtonTarget(QString item_name);   //button被点击，然后做出进路相应的逻辑
    void slotcacelbuttonselect(QString item_name);
    void slotBUttonStopFlash(QString item_name);  //停止闪烁，与topscene的联锁逻辑有关信号相对一个


protected:
    void timerEvent(QTimerEvent * event) override; //

private:
    int  enable_falsh; // 0表不启动闪烁，1表示启动闪速，通过外部信号传入
    bool flagcolor;    // 不断变化的指示灯颜色标志位
    QString name;       //该button的名称

public:
    void changeFlash(int  enable_falsh);
    void setButtonName(QString &name);
};

#endif // BUTTONWIDGET_H
