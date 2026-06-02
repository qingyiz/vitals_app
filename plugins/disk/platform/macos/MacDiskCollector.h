#pragma once

#include "IDiskCollector.h"

#include <QStorageInfo>

namespace Vitals {

/**
 * \if ENGLISH
 * @brief macOS disk collector backed by Qt mounted-volume metadata
 *
 * Enumerates mounted macOS volumes, including external drives under
 * /Volumes after the operating system has mounted them, and normalizes
 * capacity data for the disk monitor capability.
 * \endif
 *
 * \if CHINESE
 * @brief 基于 Qt 已挂载卷元数据的 macOS 磁盘采集器
 *
 * 枚举 macOS 已挂载卷，包括系统完成挂载后出现在 /Volumes 下的外接硬盘，
 * 并为磁盘监控 capability 归一化容量数据。
 * \endif
 */
class MacDiskCollector : public IDiskCollector
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
