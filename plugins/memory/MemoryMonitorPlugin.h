#pragma once

#include "IPlugin.h"

#include <QObject>
#include <memory>

namespace Vitals {

class IAppContext;
class MemoryMonitorCapability;
class MemoryPanelCapability;
class MemoryTaskbarCapability;

class MemoryMonitorPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Vitals_IPlugin_iid FILE "memory_plugin.json")
    Q_INTERFACES(Vitals::IPlugin)

public:
    explicit MemoryMonitorPlugin(QObject* parent = nullptr);
    ~MemoryMonitorPlugin() override;

    PluginMetaInfo metaInfo() const override;
    bool initialize(IAppContext* context) override;
    void start() override;
    void stop() override;
    void shutdown() override;

    IMonitorCapability* monitorCapability() override;
    IPanelCapability* panelCapability() override;
    ITaskbarCapability* taskbarCapability() override;

private:
    IAppContext* m_context = nullptr;
    std::unique_ptr<MemoryMonitorCapability> m_monitorCapability;
    std::unique_ptr<MemoryPanelCapability> m_panelCapability;
    std::unique_ptr<MemoryTaskbarCapability> m_taskbarCapability;
};

} // namespace Vitals
