#pragma once

#include <QString>

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Snapshot model used by the system information plugin collector layer
 * \endif
 *
 * \if CHINESE
 * @brief 系统信息插件采集层使用的快照数据模型
 * \endif
 */
struct SystemInfoSnapshot
{
    QString deviceName;
    QString osVersion;
    QString cpuModel;
    QString gpuModel;
    quint64 totalMemoryBytes = 0;
    qint64 uptimeSeconds = 0;
};

/**
 * \if ENGLISH
 * @brief Platform abstraction for collecting system information snapshots
 *
 * Implementations are responsible for querying one platform-specific source of
 * truth and returning a normalized snapshot for the plugin runtime and panel.
 * \endif
 *
 * \if CHINESE
 * @brief 采集系统信息快照的平台抽象接口
 *
 * 各平台实现负责查询自身平台的真实信息来源，并返回统一格式的快照数据，
 * 供插件运行时与详情页面共同使用。
 * \endif
 */
class ISystemInfoCollector
{
public:
    virtual ~ISystemInfoCollector() = default;

    /**
     * \if ENGLISH
     * @brief Performs one platform-specific system information collection pass
     * \endif
     *
     * \if CHINESE
     * @brief 执行一次平台相关的系统信息采集
     * \endif
     */
    virtual SystemInfoSnapshot collect() = 0;
};

} // namespace Vitals
