#pragma once

#include "MetricData.h"

#include <QHash>

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Extension interface for plugins that contribute taskbar or menu-bar text
 *
 * Allows a plugin to define compact host-surface text derived from the shared
 * metric cache, along with an optional custom tooltip summary. The host owns
 * placement, persistence, and platform rendering, while the plugin owns the
 * meaning of its taskbar content.
 * \endif
 *
 * \if CHINESE
 * @brief 面向任务栏或菜单栏文本贡献能力的插件扩展接口
 *
 * 该接口允许插件基于共享指标缓存定义紧凑的宿主表面显示文本，并可选提供
 * 自定义 tooltip 摘要。宿主负责位置、持久化与跨平台渲染，插件负责自己在
 * 任务栏中到底显示什么内容。
 * \endif
 */
class ITaskbarDisplayPlugin
{
public:
    virtual ~ITaskbarDisplayPlugin() = default;

    /**
     * \if ENGLISH
     * @brief Returns the compact text segment rendered in the taskbar surface
     * \endif
     *
     * \if CHINESE
     * @brief 返回渲染在任务栏表面中的紧凑文本片段
     * \endif
     */
    virtual QString taskbarDisplayText(const QHash<QString, MetricValue>& latestValues) const = 0;

    /**
     * \if ENGLISH
     * @brief Returns an optional tooltip line for this plugin's taskbar segment
     * \endif
     *
     * \if CHINESE
     * @brief 返回该插件任务栏片段对应的可选 tooltip 文本
     * \endif
     */
    virtual QString taskbarDisplayTooltip(const QHash<QString, MetricValue>& latestValues) const = 0;

    /**
     * \if ENGLISH
     * @brief Returns whether taskbar display is enabled by default for the plugin
     * \endif
     *
     * \if CHINESE
     * @brief 返回该插件的任务栏显示是否默认启用
     * \endif
     */
    virtual bool isTaskbarDisplayEnabledByDefault() const
    {
        return true;
    }
};

} // namespace Vitals

#define Vitals_ITaskbarDisplayPlugin_iid "com.vitals.plugin.ITaskbarDisplayPlugin/1.0"
Q_DECLARE_INTERFACE(Vitals::ITaskbarDisplayPlugin, Vitals_ITaskbarDisplayPlugin_iid)
