#pragma once

#include <memory>

namespace Vitals {

class IMemoryCollector;

/**
 * \if ENGLISH
 * @brief Creates the memory collector implementation for the current platform
 *
 * Keeps platform selection in one narrow dispatch point so the memory plugin
 * shell and capability classes remain free of OS-specific branches.
 * \endif
 *
 * \if CHINESE
 * @brief 为当前平台创建内存采集器实现
 *
 * 将平台选择集中在一个窄分发点中，使内存插件外壳和 capability 类不需要
 * 包含操作系统分支。
 * \endif
 */
class MemoryCollectorFactory
{
public:
    /**
     * \if ENGLISH
     * @brief Returns a collector for the active OS, or nullptr if unsupported
     * \endif
     *
     * \if CHINESE
     * @brief 返回当前系统对应的采集器；不支持时返回 nullptr
     * \endif
     */
    static std::unique_ptr<IMemoryCollector> create();
};

} // namespace Vitals
