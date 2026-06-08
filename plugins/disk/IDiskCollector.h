#pragma once

#include <QList>
#include <QString>

namespace Vitals {

struct DiskInfo
{
    QString rootPath;
    QString displayName;
    QString device;
    QString fileSystemType;
    bool isReady = false;
    bool isReadOnly = false;
    bool isRoot = false;
    bool isExternalCandidate = false;
    qint64 bytesTotal = 0;
    qint64 bytesFree = 0;
    qint64 bytesAvailable = 0;
    double activityPercent = -1.0;
};

struct DiskSnapshot
{
    QList<DiskInfo> disks;
    DiskInfo selectedDisk;
    QString selectedRootPath;
    int diskCount = 0;
};

/**
 * \if ENGLISH
 * @brief Collects mounted storage volumes for the disk monitor plugin
 *
 * Implementations enumerate host-visible mounted volumes, including external
 * disks once the operating system has mounted them, and return normalized
 * capacity metadata for the monitor capability.
 * \endif
 *
 * \if CHINESE
 * @brief 为磁盘监控插件采集已挂载存储卷
 *
 * 实现类负责枚举宿主系统可见的已挂载卷，包括操作系统挂载后的外接硬盘，
 * 并向监控 capability 返回统一的容量与挂载信息。
 * \endif
 */
class IDiskCollector
{
public:
    virtual ~IDiskCollector() = default;

    /**
     * \if ENGLISH
     * @brief Initializes collector state before periodic sampling starts
     * \endif
     *
     * \if CHINESE
     * @brief 在周期性采样开始前初始化采集器状态
     * \endif
     */
    virtual bool initialize() = 0;

    /**
     * \if ENGLISH
     * @brief Returns a snapshot for all mounted disks and the selected disk
     * @param selectedRootPath Preferred mounted root path, or empty for auto
     * \endif
     *
     * \if CHINESE
     * @brief 返回所有已挂载磁盘及当前选中磁盘的快照
     * @param selectedRootPath 首选挂载根路径，空字符串表示自动选择
     * \endif
     */
    virtual DiskSnapshot collect(const QString& selectedRootPath) = 0;
};

} // namespace Vitals
