#pragma once

#include "ICpuCollector.h"

#include <QList>

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Windows implementation of CPU usage collection for the CPU plugin
 *
 * Keeps the previous processor performance sample so usage can be calculated
 * from tick deltas without blocking the UI thread.
 * \endif
 *
 * \if CHINESE
 * @brief CPU 插件在 Windows 平台上的使用率采集实现
 *
 * 保存上一次处理器性能采样，通过 tick 差值计算使用率，避免阻塞 UI 线程。
 * \endif
 */
class WindowsCpuCollector : public ICpuCollector
{
public:
    struct CpuTicks
    {
        quint64 idle = 0;
        quint64 kernel = 0;
        quint64 user = 0;
    };

    bool initialize() override;
    CpuSnapshot collect() override;
    int logicalCoreCount() const override;
    QString cpuName() const override;

private:
    bool sampleTicks(QList<CpuTicks>& ticks) const;

    QString m_cpuName;
    int m_logicalCoreCount = 0;
    QList<CpuTicks> m_previousTicks;
};

} // namespace Vitals
