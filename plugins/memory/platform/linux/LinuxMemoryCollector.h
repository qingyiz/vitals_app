#pragma once

#include "IMemoryCollector.h"

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Linux physical memory collector backed by /proc/meminfo
 *
 * Reads kernel-provided memory counters and maps them to the plugin's unified
 * memory snapshot model.
 * \endif
 *
 * \if CHINESE
 * @brief 基于 /proc/meminfo 的 Linux 物理内存采集器
 *
 * 读取内核提供的内存计数器，并转换为插件统一的内存快照模型。
 * \endif
 */
class LinuxMemoryCollector : public IMemoryCollector
{
public:
    /**
     * \if ENGLISH
     * @brief Verifies that /proc/meminfo is readable and contains total memory
     * \endif
     *
     * \if CHINESE
     * @brief 验证 /proc/meminfo 可读取且包含总内存
     * \endif
     */
    bool initialize() override;

    /**
     * \if ENGLISH
     * @brief Collects total, used, available, and usage percentage values
     * \endif
     *
     * \if CHINESE
     * @brief 采集总量、已用、可用和使用率指标
     * \endif
     */
    MemorySnapshot collect() override;
};

} // namespace Vitals
