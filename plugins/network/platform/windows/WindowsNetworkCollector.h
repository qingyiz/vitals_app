#pragma once

#include "INetworkCollector.h"

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Windows network collector backed by the IP Helper API
 *
 * Reads cumulative byte counters from active non-loopback adapters and keeps
 * previous totals locally so the monitor capability can publish throughput
 * without depending on Windows-specific code.
 * \endif
 *
 * \if CHINESE
 * @brief 基于 IP Helper API 的 Windows 网络采集器
 *
 * 从活动的非回环网卡读取累计字节计数，并在采集器内部保存上一次总量，
 * 让监控 capability 可以发布吞吐速率且不依赖 Windows 平台代码。
 * \endif
 */
class WindowsNetworkCollector : public INetworkCollector
{
public:
    /**
     * \if ENGLISH
     * @brief Captures the initial adapter counter snapshot
     * \endif
     *
     * \if CHINESE
     * @brief 采集初始网卡计数快照
     * \endif
     */
    bool initialize() override;

    /**
     * \if ENGLISH
     * @brief Collects adapter names, cumulative counters, and calculated rates
     * \endif
     *
     * \if CHINESE
     * @brief 采集网卡名称、累计计数以及计算后的速率
     * \endif
     */
    NetworkSnapshot collect() override;

private:
    struct Totals
    {
        quint64 receivedBytes = 0;
        quint64 transmittedBytes = 0;
        QStringList activeInterfaces;
        QString primaryInterface;
    };

    bool sampleTotals(Totals& totals) const;

    Totals m_previousTotals;
    qint64 m_previousSampleMs = 0;
};

} // namespace Vitals
