#include "DiskCollectorFactory.h"

#include "IDiskCollector.h"
#include "QtStorageInfoDiskCollector.h"

namespace Vitals {

std::unique_ptr<IDiskCollector> DiskCollectorFactory::create()
{
    return std::make_unique<QtStorageInfoDiskCollector>();
}

} // namespace Vitals
