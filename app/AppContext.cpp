#include "AppContext.h"

#include "config/ConfigManager.h"
#include "metric/MetricCenter.h"

namespace Vitals {

AppContext::AppContext(MetricCenter* metricCenter, ConfigManager* configManager)
    : m_metricCenter(metricCenter)
    , m_configManager(configManager)
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

} // namespace Vitals

