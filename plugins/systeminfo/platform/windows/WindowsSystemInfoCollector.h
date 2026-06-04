#pragma once

#include "ISystemInfoCollector.h"

namespace Vitals {

class WindowsSystemInfoCollector : public ISystemInfoCollector
{
public:
    SystemInfoSnapshot collect() override;

private:
    SystemInfoSnapshot m_staticSnapshot;
    bool m_hasStaticSnapshot = false;
};

} // namespace Vitals
