#ifndef MENUPUSHBUTTON_H
#define MENUPUSHBUTTON_H

#include <QObject>
#include <QPushButton>
#include <QTime>
#include <QTimer>


//
class MenuPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MenuPushButton(const QString &text, QWidget *parent = nullptr);
    ~MenuPushButton();


public slots:
    void  buttonClicked();
    void  timeoutjob();              //清除按钮发送的clicked消息会被这个信号槽接收
    void  exterClearButtonClicked();  //外部clear清除按钮发送过来的清除指令

private:
   int clearflag;    //是否有其它信号驱动该按钮动作，默认情况下， 清除按钮发送过来的事件
                //按钮在按下之后的15s，如果没有其它关联动作，则按钮回复之前的颜色，并再次可以接收clicked事件
   QTime the_last_clicked_time;
   int jobdoneflag;   //该按钮所对应的工作是否完成
   QColor bgColor;

};

#endif // MENUPUSHBUTTON_H
