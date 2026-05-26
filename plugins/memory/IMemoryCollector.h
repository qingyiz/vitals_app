#pragma once

#include <QtGlobal>

namespace Vitals {

struct MemorySnapshot
{
    quint64 totalBytes = 0;
    quint64 usedBytes = 0;
    quint64 freeBytes = 0;
    double usagePercent = 0.0;
};

class IMemoryCollector
{
public:
    virtual ~IMemoryCollector() = default;

    virtual bool initialize() = 0;
    virtual MemorySnapshot collect() = 0;
};

} // namespace Vitals
