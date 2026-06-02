#pragma once

#include "IDiskCollector.h"

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Windows disk collector backed by Win32 mounted drive enumeration
 *
 * Enumerates ready local drive roots with Win32 APIs, including USB and other
 * external drives after Windows assigns a mounted drive letter.
 * \endif
 *
 * \if CHINESE
 * @brief 基于 Win32 已挂载盘符枚举的 Windows 磁盘采集器
 *
 * 通过 Win32 API 枚举可用的本地盘符，并在 Windows 分配盘符后包含 USB 等外接硬盘。
 * \endif
 */
class WindowsDiskCollector : public IDiskCollector
{
public:
    bool initialize() override;
    DiskSnapshot collect(const QString& selectedRootPath) override;

private:
    static DiskInfo collectDrive(const QString& rootPath);
    static QString normalizedRootPath(const QString& path);
    static QString systemRootPath();
    static QString queryDeviceName(const QString& rootPath);
    static bool isExternalDrive(const QString& rootPath, unsigned int driveType);
};

} // namespace Vitals
