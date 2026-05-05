#pragma once

#include "IPlugin.h"
#include "MetricData.h"

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Extension interface for plugins that publish monitoring metrics
 *
 * Adds metric descriptors and collection interval control on top of the base
 * plugin lifecycle so the host can understand and coordinate monitor plugins.
 * \endif
 *
 * \if CHINESE
 * @brief 面向监控型插件的扩展接口
 *
 * 在基础插件生命周期之上增加指标描述能力与采集周期控制，使宿主能够理解
 * 并调度监控型插件。
 * \endif
 */
class IMonitorPlugin : public virtual IPlugin
{
public:
    ~IMonitorPlugin() override = default;

    /**
     * \if ENGLISH
     * @brief Declares the metrics that this plugin may publish
     * \endif
     *
     * \if CHINESE
     * @brief 声明该插件可能发布的指标集合
     * \endif
     */
    virtual QList<MetricDescriptor> metricDescriptors() const = 0;

    /**
     * \if ENGLISH
     * @brief Returns the plugin's preferred default collection interval
     * \endif
     *
     * \if CHINESE
     * @brief 返回插件默认建议的采集周期
     * \endif
     */
    virtual int defaultIntervalMs() const = 0;

    /**
     * \if ENGLISH
     * @brief Updates the collection interval used by the plugin runtime
     * \endif
     *
     * \if CHINESE
     * @brief 更新插件运行时使用的采集周期
     * \endif
     */
    virtual void setIntervalMs(int intervalMs) = 0;
};

} // namespace Vitals

#define Vitals_IMonitorPlugin_iid "com.vitals.plugin.IMonitorPlugin/1.0"
Q_DECLARE_INTERFACE(Vitals::IMonitorPlugin, Vitals_IMonitorPlugin_iid)
