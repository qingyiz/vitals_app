#pragma once

#include "INetworkCollector.h"

namespace Vitals {

class MacNetworkCollector : public INetworkCollector
{
public:
    bool initialize() override;
    NetworkSnapshot collect() override;

private:
    struct Totals
    {
        quint64 receivedBytes = 0;
        quint64 transmittedBytes = 0;
        QStringList activeInterfaces;
        QString primaryInterface;
    };

    bool sampleTotals(Totals& totals) const;

    Totals m_previousTotals;
    qint64 m_previousSampleMs = 0;
};

} // namespace Vitals
