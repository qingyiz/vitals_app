#pragma once

#include <memory>

namespace Vitals {

class IMemoryCollector;

class MemoryCollectorFactory
{
public:
    static std::unique_ptr<IMemoryCollector> create();
};

} // namespace Vitals
