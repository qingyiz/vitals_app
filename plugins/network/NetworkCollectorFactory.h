#pragma once

#include <memory>

namespace Vitals {

class INetworkCollector;

class NetworkCollectorFactory
{
public:
    static std::unique_ptr<INetworkCollector> create();
};

} // namespace Vitals
