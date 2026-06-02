#pragma once

#include "ITaskbarCapability.h"

namespace Vitals {

class DiskMonitorCapability;
class IAppContext;

/**
 * \if ENGLISH
 * @brief Menu bar/taskbar summary provider for the selected disk
 * \endif
 *
 * \if CHINESE
 * @brief 为当前选中磁盘提供菜单栏/任务栏摘要
 * \endif
 */
class DiskTaskbarCapability : public ITaskbarCapability
{
public:
    DiskTaskbarCapability(const DiskMonitorCapability* monitorCapability, IAppContext* context);

    QString displayText(const QHash<QString, MetricValue>& latestValues) const override;
    QString tooltip(const QHash<QString, MetricValue>& latestValues) const override;
    bool isEnabledByDefault() const override;
    TaskbarDetailContent detailContent(const QHash<QString, MetricValue>& latestValues) const override;

private:
    QString text(const QString& key, const QString& fallback) const;

    const DiskMonitorCapability* m_monitorCapability = nullptr;
    IAppContext* m_context = nullptr;
};

} // namespace Vitals
