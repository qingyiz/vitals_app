#include "platform/macos/MacDiskCollector.h"

#include <QDir>
#include <QStorageInfo>

#include <algorithm>

namespace Vitals {

bool MacDiskCollector::initialize()
{
    return true;
}

DiskSnapshot MacDiskCollector::collect(const QString& selectedRootPath)
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

DiskInfo MacDiskCollector::toDiskInfo(const QStorageInfo& storage)
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

QString MacDiskCollector::normalizedRootPath(const QString& path)
{
    return QDir::cleanPath(QDir::fromNativeSeparators(path)).toLower();
}

bool MacDiskCollector::isLikelyExternalVolume(const QString& rootPath)
{
    return normalizedRootPath(rootPath).startsWith(QStringLiteral("/volumes/"));
}

} // namespace Vitals
