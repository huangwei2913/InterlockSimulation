#pragma once

#include <QObject>
#include "MenuPushButton.h"
class MenuPushButtonManager : public QObject
{
    Q_OBJECT

public:
    explicit MenuPushButtonManager(QObject* parent = nullptr);
    ~MenuPushButtonManager();

    void addMenuPushButton(MenuPushButton* button);

signals:
    //void menuButtonClicked(MenuPushButton* button);
    void menuButtonClicked(QString buttonName);


private slots:
    void handleMenuButtonClicked(MenuPushButton* button);

private:
    QList<MenuPushButton*> menuButtons;
};