#pragma once

#include <memory>

namespace Vitals {

class ISystemInfoCollector;

/**
 * \if ENGLISH
 * @brief Factory that creates the platform-specific system information collector
 * \endif
 *
 * \if CHINESE
 * @brief 用于创建平台相关系统信息采集器的工厂
 * \endif
 */
class SystemInfoCollectorFactory
{
public:
    /**
     * \if ENGLISH
     * @brief Creates the collector implementation matching the current target
     * \endif
     *
     * \if CHINESE
     * @brief 创建与当前目标平台匹配的采集器实现
     * \endif
     */
    static std::unique_ptr<ISystemInfoCollector> create();
};

} // namespace Vitals
