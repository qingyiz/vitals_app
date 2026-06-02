#include "QtStorageInfoDiskCollector.h"

#include <QDir>
#include <QStorageInfo>

#include <algorithm>

namespace Vitals {

bool QtStorageInfoDiskCollector::initialize()
{
    return true;
}

DiskSnapshot QtStorageInfoDiskCollector::collect(const QString& selectedRootPath)
{
    QList<DiskInfo> disks;
    const QList<QStorageInfo> mountedVolumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo& storage : mountedVolumes) {
        if (!storage.isValid() || !storage.isReady() || storage.bytesTotal() <= 0) {
            continue;
        }
        disks.append(toDiskInfo(storage));
    }

    std::sort(disks.begin(), disks.end(), [](const DiskInfo& left, const DiskInfo& right) {
        if (left.isRoot != right.isRoot) {
            return left.isRoot;
        }
        return QString::localeAwareCompare(left.displayName, right.displayName) < 0;
    });

    DiskSnapshot snapshot;
    snapshot.disks = disks;
    snapshot.diskCount = disks.size();

    const QString normalizedSelection = normalizedRootPath(selectedRootPath);
    for (const DiskInfo& disk : disks) {
        if (!normalizedSelection.isEmpty() && normalizedRootPath(disk.rootPath) == normalizedSelection) {
            snapshot.selectedDisk = disk;
            snapshot.selectedRootPath = disk.rootPath;
            return snapshot;
        }
    }

    if (!disks.isEmpty()) {
        snapshot.selectedDisk = disks.first();
        snapshot.selectedRootPath = disks.first().rootPath;
    }
    return snapshot;
}

DiskInfo QtStorageInfoDiskCollector::toDiskInfo(const QStorageInfo& storage)
{
    DiskInfo info;
    info.rootPath = QDir::toNativeSeparators(storage.rootPath());
    info.displayName = storage.displayName().trimmed();
    if (info.displayName.isEmpty()) {
        info.displayName = info.rootPath;
    }
    info.device = QString::fromLocal8Bit(storage.device());
    info.fileSystemType = QString::fromLocal8Bit(storage.fileSystemType());
    info.isReady = storage.isReady();
    info.isReadOnly = storage.isReadOnly();
    info.isRoot = storage.isRoot();
    info.isExternalCandidate = isLikelyExternalVolume(storage.rootPath());
    info.bytesTotal = storage.bytesTotal();
    info.bytesFree = storage.bytesFree();
    info.bytesAvailable = storage.bytesAvailable();
    return info;
}

QString QtStorageInfoDiskCollector::normalizedRootPath(const QString& path)
{
    return QDir::cleanPath(QDir::fromNativeSeparators(path)).toLower();
}

bool QtStorageInfoDiskCollector::isLikelyExternalVolume(const QString& rootPath)
{
    const QString path = normalizedRootPath(rootPath);
#if defined(Q_OS_MAC)
    return path.startsWith(QStringLiteral("/volumes/"));
#elif defined(Q_OS_WIN)
    return path.size() >= 2 && path.at(1) == QLatin1Char(':') && !path.startsWith(QStringLiteral("c:"));
#elif defined(Q_OS_LINUX)
    return path.startsWith(QStringLiteral("/media/"))
        || path.startsWith(QStringLiteral("/run/media/"))
        || path.startsWith(QStringLiteral("/mnt/"));
#else
    return false;
#endif
}

} // namespace Vitals
