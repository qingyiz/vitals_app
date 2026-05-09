#pragma once

#include "IMonitorPlugin.h"
#include "INetworkCollector.h"
#include "IPanelPlugin.h"
#include "ITaskbarDetailPlugin.h"
#include "ITaskbarDisplayPlugin.h"

#include <QObject>
#include <QPointer>
#include <memory>

class QTimer;

namespace Vitals {

class IAppContext;
class NetworkPanelWidget;

class NetworkMonitorPlugin : public QObject, public IMonitorPlugin, public IPanelPlugin, public ITaskbarDisplayPlugin, public ITaskbarDetailPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Vitals_IPlugin_iid FILE "network_plugin.json")
    Q_INTERFACES(Vitals::IPlugin Vitals::IMonitorPlugin Vitals::IPanelPlugin Vitals::ITaskbarDisplayPlugin Vitals::ITaskbarDetailPlugin)

public:
    explicit NetworkMonitorPlugin(QObject* parent = nullptr);
    ~NetworkMonitorPlugin() override;

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
    void publishSnapshot(const NetworkSnapshot& snapshot) const;

    IAppContext* m_context = nullptr;
    std::unique_ptr<INetworkCollector> m_collector;
    QTimer* m_timer = nullptr;
    int m_intervalMs = 2000;
    NetworkSnapshot m_lastSnapshot;
    QPointer<NetworkPanelWidget> m_panel;
};

} // namespace Vitals
