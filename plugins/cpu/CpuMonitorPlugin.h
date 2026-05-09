#pragma once

#include "IPlugin.h"

#include <QObject>
#include <memory>

namespace Vitals {

class IAppContext;
class CpuMonitorCapability;
class CpuPanelCapability;
class CpuTaskbarCapability;
class CpuSettingsCapability;

class CpuMonitorPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Vitals_IPlugin_iid FILE "cpu_plugin.json")
    Q_INTERFACES(Vitals::IPlugin)

public:
    explicit CpuMonitorPlugin(QObject* parent = nullptr);
    ~CpuMonitorPlugin() override;

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
    std::unique_ptr<CpuMonitorCapability> m_monitorCapability;
    std::unique_ptr<CpuPanelCapability> m_panelCapability;
    std::unique_ptr<CpuTaskbarCapability> m_taskbarCapability;
    std::unique_ptr<CpuSettingsCapability> m_settingsCapability;
};

} // namespace Vitals
