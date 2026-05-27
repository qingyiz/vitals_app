#pragma once

#include "IAppContext.h"

namespace Vitals {

class ConfigManager;
class LanguageManager;
class MetricCenter;

/**
 * \if ENGLISH
 * @brief Concrete host context implementation exposed to loaded plugins
 *
 * Wraps framework-owned services such as MetricCenter and ConfigManager behind
 * the stable IAppContext interface so plugins can consume host capabilities
 * without depending on MainWindow or other UI implementation details.
 * \endif
 *
 * \if CHINESE
 * @brief 暴露给已加载插件的宿主上下文具体实现
 *
 * 该类将 MetricCenter、ConfigManager 等框架拥有的服务封装在稳定的
 * IAppContext 接口之后，使插件可以使用宿主能力，而无需依赖 MainWindow
 * 或其他 UI 实现细节。
 * \endif
 */
class AppContext : public IAppContext
{
public:
    /// Binds framework services that will be exposed through IAppContext.
    AppContext(MetricCenter* metricCenter, ConfigManager* configManager, LanguageManager* languageManager);

    /// Returns the shared metric publication sink.
    IMetricSink* metricSink() const override;

    /// Returns the resolved configuration path of a plugin.
    QString configPathForPlugin(const QString& pluginId) const override;

    /// Resolves translated host/plugin UI text through the active language catalog.
    QString translate(const QString& key, const QString& fallback = QString()) const override;

    /// Returns the active language code.
    QString currentLanguage() const override;

private:
    MetricCenter* m_metricCenter = nullptr;
    ConfigManager* m_configManager = nullptr;
    LanguageManager* m_languageManager = nullptr;
};

} // namespace Vitals
