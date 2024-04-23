#pragma once

#include <QEvent>

class SwitchReverseOperationEvent : public QEvent {
public:
    SwitchReverseOperationEvent(QObject* parent = nullptr);
    QObject* parent() const { return m_parent; }

private:
    QObject* m_parent;
};