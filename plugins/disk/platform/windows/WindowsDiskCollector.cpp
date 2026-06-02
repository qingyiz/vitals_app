#include "platform/windows/WindowsDiskCollector.h"

#include <QDir>
#include <QStorageInfo>
#include <QVector>
#include <QtGlobal>

#include <algorithm>
#include <cwchar>

#define NOMINMAX
#include <windows.h>
#include <winioctl.h>

namespace Vitals {

namespace {

bool shouldIncludeDriveType(unsigned int driveType)
{
    return driveType == DRIVE_FIXED
        || driveType == DRIVE_REMOVABLE
        || driveType == DRIVE_RAMDISK;
}

QString driveTypeName(unsigned int driveType)
{
    switch (driveType) {
    case DRIVE_FIXED:
        return QStringLiteral("fixed");
    case DRIVE_REMOVABLE:
        return QStringLiteral("removable");
    case DRIVE_RAMDISK:
        return QStringLiteral("ramdisk");
    default:
        return QStringLiteral("unknown");
    }
}

bool isExternalBusType(STORAGE_BUS_TYPE busType)
{
    return busType == BusTypeUsb
        || busType == BusTypeSd
        || busType == BusTypeMmc
        || busType == BusType1394;
}

} // namespace

bool WindowsDiskCollector::initialize()
{
    return true;
}

DiskSnapshot WindowsDiskCollector::collect(const QString& selectedRootPath)
{
    QList<DiskInfo> disks;

    const DWORD requiredLength = GetLogicalDriveStringsW(0, nullptr);
    if (requiredLength == 0) {
        return {};
    }

    QVector<wchar_t> buffer(static_cast<int>(requiredLength) + 1);
    const DWORD writtenLength = GetLogicalDriveStringsW(requiredLength, buffer.data());
    if (writtenLength == 0) {
        return {};
    }

    for (const wchar_t* cursor = buffer.constData(); *cursor != L'\0'; cursor += wcslen(cursor) + 1) {
        const QString rootPath = QString::fromWCharArray(cursor);
        const unsigned int driveType = GetDriveTypeW(reinterpret_cast<LPCWSTR>(rootPath.utf16()));
        if (!shouldIncludeDriveType(driveType)) {
            continue;
        }

        DiskInfo disk = collectDrive(rootPath);
        if (disk.isReady && disk.bytesTotal > 0) {
            disks.append(disk);
        }
    }

    std::sort(disks.begin(), disks.end(), [](const DiskInfo& left, const DiskInfo& right) {
        if (left.isRoot != right.isRoot) {
            return left.isRoot;
        }
        if (left.isExternalCandidate != right.isExternalCandidate) {
            return !left.isExternalCandidate;
        }
        return QString::localeAwareCompare(left.rootPath, right.rootPath) < 0;
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

DiskInfo WindowsDiskCollector::collectDrive(const QString& rootPath)
{
    DiskInfo info;
    info.rootPath = QDir::toNativeSeparators(rootPath);

    ULARGE_INTEGER availableToCaller = {};
    ULARGE_INTEGER totalBytes = {};
    ULARGE_INTEGER freeBytes = {};
    if (!GetDiskFreeSpaceExW(reinterpret_cast<LPCWSTR>(rootPath.utf16()),
            &availableToCaller,
            &totalBytes,
            &freeBytes)) {
        return info;
    }

    wchar_t volumeName[MAX_PATH + 1] = {};
    wchar_t fileSystemName[MAX_PATH + 1] = {};
    DWORD flags = 0;
    const bool hasVolumeInfo = GetVolumeInformationW(reinterpret_cast<LPCWSTR>(rootPath.utf16()),
        volumeName,
        MAX_PATH + 1,
        nullptr,
        nullptr,
        &flags,
        fileSystemName,
        MAX_PATH + 1);

    const unsigned int driveType = GetDriveTypeW(reinterpret_cast<LPCWSTR>(rootPath.utf16()));
    info.displayName = hasVolumeInfo ? QString::fromWCharArray(volumeName).trimmed() : QString();
    if (info.displayName.isEmpty()) {
        info.displayName = info.rootPath;
    }
    info.device = queryDeviceName(rootPath);
    if (info.device.isEmpty()) {
        info.device = QStringLiteral("%1 (%2)").arg(info.rootPath, driveTypeName(driveType));
    }
    info.fileSystemType = hasVolumeInfo ? QString::fromWCharArray(fileSystemName).trimmed() : QString();
    info.isReady = true;
    info.isReadOnly = (flags & FILE_READ_ONLY_VOLUME) != 0;
    info.isRoot = normalizedRootPath(info.rootPath) == normalizedRootPath(systemRootPath());
    info.isExternalCandidate = isExternalDrive(rootPath, driveType);
    info.bytesTotal = static_cast<qint64>(totalBytes.QuadPart);
    info.bytesFree = static_cast<qint64>(freeBytes.QuadPart);
    info.bytesAvailable = static_cast<qint64>(availableToCaller.QuadPart);
    return info;
}

QString WindowsDiskCollector::normalizedRootPath(const QString& path)
{
    QString normalized = QDir::toNativeSeparators(QDir::cleanPath(QDir::fromNativeSeparators(path))).toLower();
    if (normalized.size() == 2 && normalized.at(1) == QLatin1Char(':')) {
        normalized.append(QLatin1Char('\\'));
    }
    return normalized;
}

QString WindowsDiskCollector::systemRootPath()
{
    const QString systemDrive = qEnvironmentVariable("SystemDrive");
    if (systemDrive.size() >= 2) {
        return systemDrive.left(2) + QStringLiteral("\\");
    }

    wchar_t windowsDirectory[MAX_PATH + 1] = {};
    if (GetWindowsDirectoryW(windowsDirectory, MAX_PATH + 1) > 0) {
        const QString path = QString::fromWCharArray(windowsDirectory);
        if (path.size() >= 2) {
            return path.left(2) + QStringLiteral("\\");
        }
    }
    return QStorageInfo::root().rootPath();
}

QString WindowsDiskCollector::queryDeviceName(const QString& rootPath)
{
    const QString driveName = normalizedRootPath(rootPath).left(2);
    wchar_t targetPath[MAX_PATH + 1] = {};
    const DWORD length = QueryDosDeviceW(reinterpret_cast<LPCWSTR>(driveName.utf16()), targetPath, MAX_PATH + 1);
    return length > 0 ? QString::fromWCharArray(targetPath) : QString();
}

bool WindowsDiskCollector::isExternalDrive(const QString& rootPath, unsigned int driveType)
{
    if (driveType == DRIVE_REMOVABLE) {
        return true;
    }

    const QString normalized = normalizedRootPath(rootPath);
    if (normalized.size() < 2) {
        return false;
    }

    const QString volumePath = QStringLiteral("\\\\.\\%1").arg(normalized.left(2));
    HANDLE handle = CreateFileW(reinterpret_cast<LPCWSTR>(volumePath.utf16()),
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    STORAGE_PROPERTY_QUERY query = {};
    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;

    BYTE descriptorBuffer[1024] = {};
    DWORD bytesReturned = 0;
    const BOOL ok = DeviceIoControl(handle,
        IOCTL_STORAGE_QUERY_PROPERTY,
        &query,
        sizeof(query),
        descriptorBuffer,
        sizeof(descriptorBuffer),
        &bytesReturned,
        nullptr);
    CloseHandle(handle);

    if (!ok || bytesReturned < sizeof(STORAGE_DEVICE_DESCRIPTOR)) {
        return false;
    }

    const auto* descriptor = reinterpret_cast<const STORAGE_DEVICE_DESCRIPTOR*>(descriptorBuffer);
    return isExternalBusType(descriptor->BusType);
}

} // namespace Vitals
