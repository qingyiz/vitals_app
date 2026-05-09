#pragma once

#include <QString>
#include <QStringList>

namespace Vitals {

struct NetworkSnapshot
{
    QString primaryInterface;
    QStringList activeInterfaces;
    quint64 totalReceivedBytes = 0;
    quint64 totalTransmittedBytes = 0;
    double receiveBytesPerSecond = 0.0;
    double transmitBytesPerSecond = 0.0;
};

class INetworkCollector
{
public:
    virtual ~INetworkCollector() = default;

    virtual bool initialize() = 0;
    virtual NetworkSnapshot collect() = 0;
};

} // namespace Vitals
