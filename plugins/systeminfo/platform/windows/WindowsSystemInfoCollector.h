#pragma once

#include "ISystemInfoCollector.h"

namespace Vitals {

class WindowsSystemInfoCollector : public ISystemInfoCollector
{
public:
    SystemInfoSnapshot collect() override;
};

} // namespace Vitals
