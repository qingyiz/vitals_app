#pragma once

#include "IPlugin.h"

#include <QObject>
#include <memory>

namespace Vitals {

class DiskMonitorCapability;
class DiskPanelCapability;
class DiskSettingsCapability;
class DiskTaskbarCapability;
class IAppContext;

/**
 * \if ENGLISH
 * @brief Plugin shell for mounted disk monitoring
 * \endif
 *
 * \if CHINESE
 * @brief 已挂载磁盘监控插件壳
 * \endif
 */
class DiskMonitorPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Vitals_IPlugin_iid FILE "disk_plugin.json")
    Q_INTERFACES(Vitals::IPlugin)

public:
    explicit DiskMonitorPlugin(QObject* parent = nullptr);
    ~DiskMonitorPlugin() override;

    PluginMetaInfo metaInfo() const override;
    bool initialize(IAppContext* context) override;
    void start() override;
    void stop() override;
    void shutdown() override;

    IMonitorCapability* monitorCapability() override;
    IPanelCapability* panelCapability() override;
    ITaskbarCapability* taskbarCapability() override;
    ISettingsCapability* settingsCapability() override;

private:
    IAppContext* m_context = nullptr;
    std::unique_ptr<DiskMonitorCapability> m_monitorCapability;
    std::unique_ptr<DiskPanelCapability> m_panelCapability;
    std::unique_ptr<DiskTaskbarCapability> m_taskbarCapability;
    std::unique_ptr<DiskSettingsCapability> m_settingsCapability;
};

} // namespace Vitals
