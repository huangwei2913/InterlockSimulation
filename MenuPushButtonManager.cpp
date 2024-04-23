#include "MenuPushButtonManager.h"

MenuPushButtonManager::MenuPushButtonManager(QObject* parent)
    : QObject(parent)
{
}

MenuPushButtonManager::~MenuPushButtonManager()
{
    // 清除管理的MenuPushButton对象

}

void MenuPushButtonManager::addMenuPushButton(MenuPushButton* button)
{
    if (button) {
        // 连接MenuPushButton对象的clickedButton信号与Manager的handleMenuButtonClicked槽函数
        connect(button, &MenuPushButton::clickedButton, this, &MenuPushButtonManager::handleMenuButtonClicked);
        menuButtons.append(button);
    }
}

void MenuPushButtonManager::handleMenuButtonClicked(MenuPushButton* button)
{
    // 处理MenuPushButton对象被点击的逻辑

    // 恢复其他MenuPushButton对象的颜色状态
    for (MenuPushButton* otherButton : menuButtons) {
        if (otherButton != button) {
            otherButton->restoreColor(otherButton);
        }
    }

    // 发送menuButtonClicked信号，通知其他对象被点击
    emit menuButtonClicked(button->getName());

}