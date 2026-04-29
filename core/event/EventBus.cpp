#include "event/EventBus.h"

namespace Vitals {

EventBus::EventBus(QObject* parent)
    : QObject(parent)
{
}

void EventBus::publish(const QString& topic, const QVariant& payload)
{
    Q_EMIT eventPublished(topic, payload);
}

} // namespace Vitals

