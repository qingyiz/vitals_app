#pragma once

#include "IPlugin.h"

#include <QObject>
#include <memory>

namespace Vitals {

class IAppContext;
class NetworkMonitorCapability;
class NetworkPanelCapability;
class NetworkTaskbarCapability;
class NetworkSettingsCapability;

class NetworkMonitorPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Vitals_IPlugin_iid FILE "network_plugin.json")
    Q_INTERFACES(Vitals::IPlugin)

public:
    explicit NetworkMonitorPlugin(QObject* parent = nullptr);
    ~NetworkMonitorPlugin() override;

    PluginMetaInfo metaInfo() const override;
    bool initialize(IAppContext* context) override;
    void start() override;
    void stop() override;
    void shutdown() override;

    IMonitorCapability* monitorCapability() override;
    IPanelCapability* panelCapability() override;
    ITaskbarCapability* taskbarCapability() override;
    ISettingsCapability* settingsCapability() override;

    IAppContext* m_context = nullptr;
    std::unique_ptr<NetworkMonitorCapability> m_monitorCapability;
    std::unique_ptr<NetworkPanelCapability> m_panelCapability;
    std::unique_ptr<NetworkTaskbarCapability> m_taskbarCapability;
    std::unique_ptr<NetworkSettingsCapability> m_settingsCapability;
};

} // namespace Vitals
