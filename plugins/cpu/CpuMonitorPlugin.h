#pragma once

#include "IMonitorPlugin.h"
#include "IPanelPlugin.h"
#include "ITaskbarDetailPlugin.h"
#include "ITaskbarDisplayPlugin.h"
#include "ICpuCollector.h"

#include <QObject>
#include <QPointer>
#include <memory>

class QTimer;

namespace Vitals {

class IAppContext;
class CpuPanelWidget;

class CpuMonitorPlugin : public QObject, public IMonitorPlugin, public IPanelPlugin, public ITaskbarDisplayPlugin, public ITaskbarDetailPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Vitals_IPlugin_iid FILE "cpu_plugin.json")
    Q_INTERFACES(Vitals::IPlugin Vitals::IMonitorPlugin Vitals::IPanelPlugin Vitals::ITaskbarDisplayPlugin Vitals::ITaskbarDetailPlugin)

public:
    explicit CpuMonitorPlugin(QObject* parent = nullptr);
    ~CpuMonitorPlugin() override;

    PluginMetaInfo metaInfo() const override;
    bool initialize(IAppContext* context) override;
    void start() override;
    void stop() override;
    void shutdown() override;

    QList<MetricDescriptor> metricDescriptors() const override;
    int defaultIntervalMs() const override;
    void setIntervalMs(int intervalMs) override;

    QString panelId() const override;
    QString panelName() const override;
    QString panelIconKey() const override;
    QWidget* createPanel(QWidget* parent = nullptr) override;

    QString taskbarDisplayText(const QHash<QString, MetricValue>& latestValues) const override;
    QString taskbarDisplayTooltip(const QHash<QString, MetricValue>& latestValues) const override;
    bool isTaskbarDisplayEnabledByDefault() const override;
    TaskbarDetailContent taskbarDetailContent(const QHash<QString, MetricValue>& latestValues) const override;

private:
    void collectAndPublish();
    void publishSnapshot(const CpuSnapshot& snapshot) const;

    IAppContext* m_context = nullptr;
    std::unique_ptr<ICpuCollector> m_collector;
    QTimer* m_timer = nullptr;
    int m_intervalMs = 2000;
    CpuSnapshot m_lastSnapshot;
    QPointer<CpuPanelWidget> m_panel;
};

} // namespace Vitals
