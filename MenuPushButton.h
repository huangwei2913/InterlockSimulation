#ifndef MENUPUSHBUTTON_H
#define MENUPUSHBUTTON_H

#include <QPushButton>
#include <QTimer>

class MenuPushButton : public QPushButton
{
    Q_OBJECT

public:
    explicit MenuPushButton(const QString& name, QWidget* parent = nullptr);
    ~MenuPushButton();

    QString getName() const { return name; }

signals:
    void clickedButton(MenuPushButton* button);

public slots:
    void handleTimeout();
    void resetColor();
    void restoreColor(MenuPushButton* button); // 修正为槽函数

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    QString name;
    QTimer* timer;
    bool isFlashing;
    bool isClicked;
};
#endif // MENUPUSHBUTTON_H
