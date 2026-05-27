#pragma once

#include <QtGlobal>

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Immutable physical memory sample produced by platform collectors
 *
 * Uses bytes for capacity values and percentage for usage so monitor, panel,
 * taskbar, and dashboard surfaces can share the same data without platform
 * conditionals.
 * \endif
 *
 * \if CHINESE
 * @brief 平台采集器生成的物理内存快照
 *
 * 容量字段统一使用字节，使用率字段统一使用百分比，使监控、面板、
 * 任务栏和 Dashboard 可以共享同一份数据而无需平台判断。
 * \endif
 */
struct MemorySnapshot
{
    quint64 totalBytes = 0;
    quint64 usedBytes = 0;
    quint64 freeBytes = 0;
    double usagePercent = 0.0;
};

/**
 * \if ENGLISH
 * @brief Platform-neutral collector contract for the memory monitor plugin
 *
 * Concrete implementations live under platform-specific directories. The
 * monitor capability owns this interface and publishes the returned snapshots
 * through the shared Metric model.
 * \endif
 *
 * \if CHINESE
 * @brief 内存监控插件的平台无关采集接口
 *
 * 具体实现位于各平台目录中。监控 capability 持有该接口，并将返回的
 * 快照通过统一 Metric 模型发布给宿主。
 * \endif
 */
class IMemoryCollector
{
public:
    virtual ~IMemoryCollector() = default;

    /**
     * \if ENGLISH
     * @brief Initializes platform resources before periodic collection starts
     * \endif
     *
     * \if CHINESE
     * @brief 在定时采集开始前初始化平台资源
     * \endif
     */
    virtual bool initialize() = 0;

    /**
     * \if ENGLISH
     * @brief Returns the latest physical memory snapshot
     * \endif
     *
     * \if CHINESE
     * @brief 返回最新的物理内存快照
     * \endif
     */
    virtual MemorySnapshot collect() = 0;
};

} // namespace Vitals
