#include "MenuPushButton.h"
#include <QPalette>
#include <QMouseEvent>

MenuPushButton::MenuPushButton(const QString& name, QWidget* parent)
    : QPushButton(name, parent), name(name), isFlashing(false), isClicked(false)
{
    // 设置默认颜色
    setStyleSheet("background-color: rgb(115, 115, 115);");

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MenuPushButton::resetColor);
}

MenuPushButton::~MenuPushButton()
{
    delete timer;
}

void MenuPushButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        isClicked = true;
        setStyleSheet("background-color: rgb(255, 0, 255);"); // 设置点击后的颜色为紫色
        timer->start(15000); // 15秒后恢复到原来的颜色
        emit clickedButton(this);
    }

    QPushButton::mousePressEvent(event);
}

void MenuPushButton::handleTimeout()
{
    if (!isClicked) {
        setStyleSheet("background-color: rgb(115, 115, 115);"); // 恢复到原来的颜色
    }
    isClicked = false;
}

void MenuPushButton::resetColor()
{
    setStyleSheet("background-color: rgb(115, 115, 115);"); // 恢复到原来的颜色
}

void MenuPushButton::restoreColor(MenuPushButton* button)
{
    if (button == this) {
        resetColor(); // 处理外部发送的恢复颜色的消息
    }
}