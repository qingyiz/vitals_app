#pragma once

#include "IMemoryCollector.h"

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Windows physical memory collector backed by GlobalMemoryStatusEx
 *
 * Keeps platform-specific Win32 querying behind the memory plugin collector
 * contract so the plugin shell and monitor capability stay platform neutral.
 * \endif
 *
 * \if CHINESE
 * @brief 基于 GlobalMemoryStatusEx 的 Windows 物理内存采集器
 *
 * 将 Win32 平台查询封装在内存插件采集接口之后，使插件外壳与监控
 * capability 保持平台无关。
 * \endif
 */
class WindowsMemoryCollector : public IMemoryCollector
{
public:
    /**
     * \if ENGLISH
     * @brief Verifies that the Windows memory API can provide a valid snapshot
     * \endif
     *
     * \if CHINESE
     * @brief 验证 Windows 内存 API 可以返回有效快照
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
