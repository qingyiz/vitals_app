#pragma once

#include "IPlugin.h"

#include <QIcon>

class QWidget;

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Extension interface for plugins that provide a QWidget-based panel
 *
 * Allows a plugin to contribute a navigable panel page to the host UI while
 * keeping ownership of page creation inside the plugin boundary.
 * \endif
 *
 * \if CHINESE
 * @brief 面向提供 QWidget 面板页面的插件扩展接口
 *
 * 允许插件向宿主 UI 提供可导航的面板页面，同时保持页面创建逻辑仍由插件
 * 自身负责。
 * \endif
 */
class IPanelPlugin : public IPlugin
{
public:
    ~IPanelPlugin() override = default;

    /**
     * \if ENGLISH
     * @brief Returns the stable navigation identifier of the panel
     * \endif
     *
     * \if CHINESE
     * @brief 返回该面板在导航中的稳定标识
     * \endif
     */
    virtual QString panelId() const = 0;

    /**
     * \if ENGLISH
     * @brief Returns the human-readable panel title shown by the host
     * \endif
     *
     * \if CHINESE
     * @brief 返回宿主展示给用户的面板名称
     * \endif
     */
    virtual QString panelName() const = 0;

    /**
     * \if ENGLISH
     * @brief Returns the semantic icon key used by host navigation
     * \endif
     *
     * \if CHINESE
     * @brief 返回宿主导航使用的语义化图标 key
     * \endif
     */
    virtual QString panelIconKey() const = 0;

    /**
     * \if ENGLISH
     * @brief Returns a custom icon override for host navigation when needed
     *
     * Most plugins should prefer panelIconKey() and let the host render a
     * consistent navigation icon. This function is a fallback for plugins that
     * genuinely need a custom-drawn icon.
     * \endif
     *
     * \if CHINESE
     * @brief 在确有需要时返回宿主导航使用的自定义图标覆盖值
     *
     * 大多数插件应优先返回 panelIconKey()，由宿主统一生成风格一致的导航
     * 图标。该函数仅作为确实需要自定义图标时的兜底接口。
     * \endif
     */
    virtual QIcon panelIcon() const
    {
        return {};
    }

    /**
     * \if ENGLISH
     * @brief Creates the plugin-owned QWidget panel instance
     * \endif
     *
     * \if CHINESE
     * @brief 创建由插件拥有逻辑的 QWidget 面板实例
     * \endif
     */
    virtual QWidget* createPanel(QWidget* parent = nullptr) = 0;
};

} // namespace Vitals

#define Vitals_IPanelPlugin_iid "com.vitals.plugin.IPanelPlugin/1.0"
Q_DECLARE_INTERFACE(Vitals::IPanelPlugin, Vitals_IPanelPlugin_iid)
