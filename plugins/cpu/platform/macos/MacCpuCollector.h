#pragma once

#include "ICpuCollector.h"

#include <QList>

namespace Vitals {

class MacCpuCollector : public ICpuCollector
{
public:
    struct CpuTicks
    {
        quint64 user = 0;
        quint64 system = 0;
        quint64 idle = 0;
        quint64 nice = 0;
    };

    bool initialize() override;
    CpuSnapshot collect() override;
    int logicalCoreCount() const override;
    QString cpuName() const override;

private:
    bool sampleTicks(QList<CpuTicks>& ticks) const;

    QString m_cpuName;
    int m_logicalCoreCount = 0;
    QList<CpuTicks> m_previousTicks;
};

} // namespace Vitals
