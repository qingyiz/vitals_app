#pragma once

#include "IDiskCollector.h"
#include "ISettingsCapability.h"

#include <QPointer>

class QComboBox;
class QLabel;
class QWidget;

namespace Vitals {

class DiskMonitorCapability;
class IAppContext;

/**
 * \if ENGLISH
 * @brief Settings capability for choosing the monitored disk
 * \endif
 *
 * \if CHINESE
 * @brief 用于选择当前监控磁盘的设置能力
 * \endif
 */
class DiskSettingsCapability : public ISettingsCapability
{
public:
    DiskSettingsCapability(DiskMonitorCapability* monitorCapability, IAppContext* context);

    QString settingsId() const override;
    QString settingsTitle() const override;
    QWidget* createSettingsWidget(QWidget* parent = nullptr) override;

    void updateSnapshot(const DiskSnapshot& snapshot);

private:
    QString text(const QString& key, const QString& fallback) const;
    QString formatDiskName(const DiskInfo& disk) const;
    void rebuildCombo();

    DiskMonitorCapability* m_monitorCapability = nullptr;
    IAppContext* m_context = nullptr;
    QPointer<QWidget> m_widget;
    QPointer<QComboBox> m_combo;
    QPointer<QLabel> m_hintLabel;
    DiskSnapshot m_lastSnapshot;
};

} // namespace Vitals
