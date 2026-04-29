#include "metric/MetricRegistry.h"

namespace Vitals {

void MetricRegistry::registerDescriptors(const QList<MetricDescriptor>& descriptors)
{
    for (const MetricDescriptor& descriptor : descriptors) {
        m_descriptors.insert(descriptor.key, descriptor);
    }
}

bool MetricRegistry::contains(const QString& key) const
{
    return m_descriptors.contains(key);
}

QList<MetricDescriptor> MetricRegistry::descriptors() const
{
    return m_descriptors.values();
}

} // namespace Vitals

