#pragma once

#include "ISystemInfoCollector.h"

namespace Vitals {

/**
 * \if ENGLISH
 * @brief macOS implementation of the system information collector
 * \endif
 *
 * \if CHINESE
 * @brief macOS 平台下的系统信息采集器实现
 * \endif
 */
class MacSystemInfoCollector : public ISystemInfoCollector
{
public:
    SystemInfoSnapshot collect() override;

private:
    SystemInfoSnapshot m_staticSnapshot;
    bool m_hasStaticSnapshot = false;
};

} // namespace Vitals
