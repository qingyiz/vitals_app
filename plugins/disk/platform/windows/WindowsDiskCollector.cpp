#include "platform/windows/WindowsDiskCollector.h"

#include <QDir>
#include <QHash>
#include <QStorageInfo>
#include <QVector>
#include <QtGlobal>

#include <algorithm>
#include <memory>
#include <cwchar>

#define NOMINMAX
#include <pdhmsg.h>
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

QString driveRootKey(const QString& rootPath)
{
    QString normalized = QDir::toNativeSeparators(QDir::cleanPath(QDir::fromNativeSeparators(rootPath))).toLower();
    if (normalized.size() == 2 && normalized.at(1) == QLatin1Char(':')) {
        normalized.append(QLatin1Char('\\'));
    }
    return normalized.left(2);
}

QString driveKeyFromPdhInstance(const QString& instanceName)
{
    const QString normalized = instanceName.toLower();
    for (int index = 0; index + 1 < normalized.size(); ++index) {
        const QChar driveLetter = normalized.at(index);
        if (driveLetter >= QLatin1Char('a')
            && driveLetter <= QLatin1Char('z')
            && normalized.at(index + 1) == QLatin1Char(':')) {
            return normalized.mid(index, 2);
        }
    }
    return {};
}

} // namespace

WindowsDiskCollector::~WindowsDiskCollector()
{
    if (m_activityQuery) {
        PdhCloseQuery(m_activityQuery);
    }
}

bool WindowsDiskCollector::initialize()
{
    initializeActivityQuery();
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

    applyActivitySamples(&disks);

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

bool WindowsDiskCollector::initializeActivityQuery()
{
    if (m_activityQueryReady) {
        return true;
    }

    PDH_HQUERY query = nullptr;
    PDH_STATUS status = PdhOpenQueryW(nullptr, 0, &query);
    if (status != ERROR_SUCCESS) {
        return false;
    }

    PDH_HCOUNTER counter = nullptr;
    status = PdhAddEnglishCounterW(query, L"\\PhysicalDisk(*)\\% Idle Time", 0, &counter);
    if (status != ERROR_SUCCESS) {
        PdhCloseQuery(query);
        return false;
    }

    m_activityQuery = query;
    m_idleTimeCounter = counter;
    m_activityQueryReady = true;
    return true;
}

void WindowsDiskCollector::applyActivitySamples(QList<DiskInfo>* disks)
{
    if (!disks || disks->isEmpty() || !initializeActivityQuery()) {
        return;
    }

    const PDH_STATUS collectStatus = PdhCollectQueryData(m_activityQuery);
    if (collectStatus != ERROR_SUCCESS) {
        return;
    }

    if (!m_hasActivityBaseline) {
        m_hasActivityBaseline = true;
        return;
    }

    DWORD bufferSize = 0;
    DWORD itemCount = 0;
    PDH_STATUS status = PdhGetFormattedCounterArrayW(m_idleTimeCounter,
        PDH_FMT_DOUBLE,
        &bufferSize,
        &itemCount,
        nullptr);
    if (status != PDH_MORE_DATA || bufferSize == 0 || itemCount == 0) {
        return;
    }

    auto buffer = std::make_unique<unsigned char[]>(bufferSize);
    auto* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer.get());
    status = PdhGetFormattedCounterArrayW(m_idleTimeCounter,
        PDH_FMT_DOUBLE,
        &bufferSize,
        &itemCount,
        items);
    if (status != ERROR_SUCCESS) {
        return;
    }

    QHash<QString, double> activityByDrive;
    for (DWORD index = 0; index < itemCount; ++index) {
        const QString instanceName = QString::fromWCharArray(items[index].szName);
        if (instanceName == QStringLiteral("_Total")) {
            continue;
        }

        const QString driveKey = driveKeyFromPdhInstance(instanceName);
        if (driveKey.isEmpty() || items[index].FmtValue.CStatus != PDH_CSTATUS_VALID_DATA) {
            continue;
        }

        const double idle = qBound(0.0, items[index].FmtValue.doubleValue, 100.0);
        activityByDrive.insert(driveKey, qBound(0.0, 100.0 - idle, 100.0));
    }

    for (DiskInfo& disk : *disks) {
        const QString driveKey = driveRootKey(disk.rootPath);
        if (activityByDrive.contains(driveKey)) {
            disk.activityPercent = activityByDrive.value(driveKey);
        }
    }
}

} // namespace Vitals
