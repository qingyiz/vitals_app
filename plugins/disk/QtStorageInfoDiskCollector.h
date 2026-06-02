#pragma once

#include "IDiskCollector.h"

#include <QStorageInfo>

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Cross-platform disk collector backed by Qt QStorageInfo
 *
 * Uses Qt's mounted-volume abstraction so fixed volumes and external drives
 * are reported through the same collector contract on Windows, macOS, and Linux.
 * \endif
 *
 * \if CHINESE
 * @brief 基于 Qt QStorageInfo 的跨平台磁盘采集器
 *
 * 使用 Qt 的已挂载卷抽象，让固定磁盘和外接硬盘在 Windows、macOS、Linux
 * 上通过同一个采集器契约返回。
 * \endif
 */
class QtStorageInfoDiskCollector : public IDiskCollector
{
public:
    bool initialize() override;
    DiskSnapshot collect(const QString& selectedRootPath) override;

private:
    static DiskInfo toDiskInfo(const QStorageInfo& storage);
    static QString normalizedRootPath(const QString& path);
    static bool isLikelyExternalVolume(const QString& rootPath);
};

} // namespace Vitals
