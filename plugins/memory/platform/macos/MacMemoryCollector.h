#pragma once

#include "IMemoryCollector.h"

namespace Vitals {

class MacMemoryCollector : public IMemoryCollector
{
public:
    bool initialize() override;
    MemorySnapshot collect() override;

private:
    quint64 m_totalBytes = 0;
};

} // namespace Vitals
