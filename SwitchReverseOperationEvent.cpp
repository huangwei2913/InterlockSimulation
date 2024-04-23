#include "SwitchReverseOperationEvent.h"

SwitchReverseOperationEvent::SwitchReverseOperationEvent(QObject* parent)
    : QEvent(static_cast<QEvent::Type>(QEvent::User + 2)), m_parent(parent)
{
}