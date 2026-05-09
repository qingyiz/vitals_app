#pragma once

#include "IPlugin.h"

#include <QObject>
#include <memory>

namespace Vitals {

class IAppContext;
class SystemInfoMonitorCapability;
class SystemInfoPanelCapability;
class SystemInfoTaskbarCapability;
class SystemInfoSettingsCapability;

/**
 * \if ENGLISH
 * @brief macOS-first system information plugin for the Vitals host
 *
 * Publishes normalized host identity metrics and provides a read-only details
 * panel. The first implementation targets macOS only, with platform support
 * declared explicitly through plugin metadata and collector factory dispatch.
 * \endif
 *
 * \if CHINESE
 * @brief 面向 Vitals 宿主的 macOS 优先系统信息插件
 *
 * 该插件发布统一格式的宿主基础系统信息指标，并提供只读详情面板。当前首版
 * 仅支持 macOS，平台支持范围通过插件元信息与采集器工厂分发明确声明。
 * \endif
 */
class SystemInfoPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Vitals_IPlugin_iid FILE "systeminfo_plugin.json")
    Q_INTERFACES(Vitals::IPlugin)

public:
    explicit SystemInfoPlugin(QObject* parent = nullptr);
    ~SystemInfoPlugin() override;

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
    std::unique_ptr<SystemInfoMonitorCapability> m_monitorCapability;
    std::unique_ptr<SystemInfoPanelCapability> m_panelCapability;
    std::unique_ptr<SystemInfoTaskbarCapability> m_taskbarCapability;
    std::unique_ptr<SystemInfoSettingsCapability> m_settingsCapability;
};

} // namespace Vitals
