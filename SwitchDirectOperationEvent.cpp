#include "SwitchDirectOperationEvent.h"

SwitchDirectOperationEvent::SwitchDirectOperationEvent(QObject* parent)
    : QEvent(static_cast<QEvent::Type>(QEvent::User + 1)), m_parent(parent)
{
}