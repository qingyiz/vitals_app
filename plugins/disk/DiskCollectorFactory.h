#pragma once

#include <memory>

namespace Vitals {

class IDiskCollector;

/**
 * \if ENGLISH
 * @brief Creates the disk collector used by DiskMonitorPlugin
 * \endif
 *
 * \if CHINESE
 * @brief 创建 DiskMonitorPlugin 使用的磁盘采集器
 * \endif
 */
class DiskCollectorFactory
{
public:
    static std::unique_ptr<IDiskCollector> create();
};

} // namespace Vitals
