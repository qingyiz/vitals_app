#pragma once

#include <memory>

namespace Vitals {

class ICpuCollector;

class CpuCollectorFactory
{
public:
    static std::unique_ptr<ICpuCollector> create();
};

} // namespace Vitals
