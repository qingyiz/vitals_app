#pragma once

#include <QList>
#include <QString>

namespace Vitals {

struct CpuSnapshot
{
    QString cpuName;
    double totalUsagePercent = 0.0;
    QList<double> perCoreUsagePercent;
    int logicalCoreCount = 0;
    int busiestCoreIndex = -1;
    double busiestCoreUsagePercent = 0.0;
};

class ICpuCollector
{
public:
    virtual ~ICpuCollector() = default;

    virtual bool initialize() = 0;
    virtual CpuSnapshot collect() = 0;
    virtual int logicalCoreCount() const = 0;
    virtual QString cpuName() const = 0;
};

} // namespace Vitals
