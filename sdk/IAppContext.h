#pragma once

#include <QString>

namespace Vitals {

class IMetricSink;

/**
 * \if ENGLISH
 * @brief Host service gateway exposed to plugins
 *
 * Provides a narrow, framework-owned entry point for plugins to access host
 * services such as metric publication and per-plugin configuration paths.
 * Plugins should depend on this abstraction instead of concrete UI classes.
 * \endif
 *
 * \if CHINESE
 * @brief 暴露给插件的宿主服务入口
 *
 * 为插件提供一个由框架控制的窄接口，用于访问宿主服务，例如指标发布能力
 * 和插件专属配置路径。插件应依赖该抽象，而不是直接依赖具体 UI 类。
 * \endif
 */
class IAppContext
{
public:
    virtual ~IAppContext() = default;

    /**
     * \if ENGLISH
     * @brief Returns the shared metric sink used to publish plugin data
     * \endif
     *
     * \if CHINESE
     * @brief 返回用于发布插件数据的统一指标接收端
     * \endif
     */
    virtual IMetricSink* metricSink() const = 0;

    /**
     * \if ENGLISH
     * @brief Resolves the configuration file path assigned to a plugin
     * @param pluginId Stable plugin identifier
     * \endif
     *
     * \if CHINESE
     * @brief 解析某个插件对应的配置文件路径
     * @param pluginId 插件稳定标识符
     * \endif
     */
    virtual QString configPathForPlugin(const QString& pluginId) const = 0;

    /**
     * \if ENGLISH
     * @brief Resolves a host or plugin translation key using the active language
     *
     * Plugins must use this entry point for user-facing static text instead of
     * hard-coding English or Chinese strings in panels, settings pages, or
     * taskbar/menu content.
     * \endif
     *
     * \if CHINESE
     * @brief 使用当前语言解析宿主或插件翻译 key
     *
     * 插件中的面板、设置页、任务栏/菜单内容等用户可见静态文案必须通过
     * 该入口获取，不应直接硬编码英文或中文。
     * \endif
     */
    virtual QString translate(const QString& key, const QString& fallback = QString()) const = 0;

    /**
     * \if ENGLISH
     * @brief Returns the currently selected language code
     * \endif
     *
     * \if CHINESE
     * @brief 返回当前选择的语言代码
     * \endif
     */
    virtual QString currentLanguage() const = 0;
};

} // namespace Vitals
