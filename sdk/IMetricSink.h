#pragma once

#include "MetricData.h"

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Host-side sink that receives metric frames from plugins
 *
 * This interface is the write-only data entry used by monitor plugins.
 * Implementations decide how to marshal cross-thread calls, cache values, and
 * forward updates to UI or persistence layers.
 * \endif
 *
 * \if CHINESE
 * @brief 宿主侧用于接收插件指标数据的写入接口
 *
 * 该接口是监控插件向框架写入数据的统一入口。具体实现负责处理跨线程投递、
 * 数据缓存，以及向 UI 或持久化层转发更新。
 * \endif
 */
class IMetricSink
{
public:
    virtual ~IMetricSink() = default;

    /**
     * \if ENGLISH
     * @brief Publishes a batch of metric values to the host application
     *
     * This function may be called from plugin worker threads. The host-side
     * implementation is responsible for thread marshalling and dispatch.
     * \endif
     *
     * \if CHINESE
     * @brief 向宿主程序发布一批指标值
     *
     * 该函数可能会从插件工作线程中调用。宿主实现负责完成线程切换与后续分发。
     * \endif
     */
    virtual void publishFrame(const MetricFrame& frame) = 0;
};

} // namespace Vitals
