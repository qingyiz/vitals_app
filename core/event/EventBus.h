#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

namespace Vitals {

class EventBus : public QObject
{
    Q_OBJECT

public:
    explicit EventBus(QObject* parent = nullptr);

    void publish(const QString& topic, const QVariant& payload = QVariant());

Q_SIGNALS:
    void eventPublished(const QString& topic, const QVariant& payload);
};

} // namespace Vitals

