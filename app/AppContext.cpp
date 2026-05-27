#include "AppContext.h"

#include "config/ConfigManager.h"
#include "language/LanguageManager.h"
#include "metric/MetricCenter.h"

namespace Vitals {

AppContext::AppContext(MetricCenter* metricCenter, ConfigManager* configManager, LanguageManager* languageManager)
    : m_metricCenter(metricCenter)
    , m_configManager(configManager)
    , m_languageManager(languageManager)
{
}

IMetricSink* AppContext::metricSink() const
{
    return m_metricCenter;
}

QString AppContext::configPathForPlugin(const QString& pluginId) const
{
    return m_configManager ? m_configManager->pluginConfigPath(pluginId) : QString();
}

QString AppContext::translate(const QString& key, const QString& fallback) const
{
    return m_languageManager ? m_languageManager->translate(key, fallback) : fallback;
}

QString AppContext::currentLanguage() const
{
    return m_languageManager ? m_languageManager->currentLanguage() : QString();
}

} // namespace Vitals
