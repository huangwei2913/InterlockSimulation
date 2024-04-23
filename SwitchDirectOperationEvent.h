#pragma once
#include <QEvent>

class SwitchDirectOperationEvent : public QEvent {
public:
    SwitchDirectOperationEvent(QObject* parent = nullptr);
    QObject* parent() const { return m_parent; }

private:
    QObject* m_parent;
};