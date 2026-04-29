#pragma once

#include "MetricData.h"

#include <QHash>

namespace Vitals {

class MetricRegistry
{
public:
    void registerDescriptors(const QList<MetricDescriptor>& descriptors);
    bool contains(const QString& key) const;
    QList<MetricDescriptor> descriptors() const;

private:
    QHash<QString, MetricDescriptor> m_descriptors;
};

} // namespace Vitals

