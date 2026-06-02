#pragma once

#include "IDiskCollector.h"

#include <QWidget>

class QComboBox;
class QHBoxLayout;
class QLabel;

namespace Vitals {

class IAppContext;
class InfoPanelWidget;

/**
 * \if ENGLISH
 * @brief Disk monitor page with mounted-volume selection
 * \endif
 *
 * \if CHINESE
 * @brief 带已挂载卷选择能力的磁盘监控页面
 * \endif
 */
class DiskPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DiskPanelWidget(IAppContext* context, QWidget* parent = nullptr);

    void applySnapshot(const DiskSnapshot& snapshot);

Q_SIGNALS:
    void diskSelected(const QString& rootPath);

private:
    QString text(const QString& key, const QString& fallback) const;
    QString formatBytes(qint64 bytes) const;
    QString formatPercent(double value) const;
    QString formatDiskName(const DiskInfo& disk) const;
    QString diskKindLabel(const DiskInfo& disk) const;
    double usagePercent(const DiskInfo& disk) const;
    qint64 usedBytes(const DiskInfo& disk) const;
    void rebuildDiskSelector(const DiskSnapshot& snapshot);

    IAppContext* m_context = nullptr;
    InfoPanelWidget* m_infoPanel = nullptr;
    QComboBox* m_diskCombo = nullptr;
    bool m_updatingCombo = false;
};

} // namespace Vitals
