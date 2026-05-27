#include "SystemInfoPlugin.h"

#include "monitor/SystemInfoMonitorCapability.h"
#include "panel/SystemInfoPanelCapability.h"
#include "settings/SystemInfoSettingsCapability.h"
#include "taskbar/SystemInfoTaskbarCapability.h"

namespace Vitals {

SystemInfoPlugin::SystemInfoPlugin(QObject* parent)
    : QObject(parent)
{
}

SystemInfoPlugin::~SystemInfoPlugin() = default;

PluginMetaInfo SystemInfoPlugin::metaInfo() const
{
    return {
        QStringLiteral("com.vitals.systeminfo"),
        QStringLiteral("System Information"),
        QStringLiteral("Cross-platform system identity and host information plugin."),
        QStringLiteral("0.1.0"),
        QStringLiteral("Vitals"),
        QStringLiteral("monitor"),
        {QStringLiteral("macos"), QStringLiteral("windows")},
        QStringLiteral("0.1.0"),
        true
    };
}

bool SystemInfoPlugin::initialize(IAppContext* context)
{
    m_context = context;

    m_monitorCapability = std::make_unique<SystemInfoMonitorCapability>();
    m_panelCapability = std::make_unique<SystemInfoPanelCapability>();
    m_taskbarCapability = std::make_unique<SystemInfoTaskbarCapability>(m_monitorCapability.get());
    m_settingsCapability = std::make_unique<SystemInfoSettingsCapability>();

    if (!m_monitorCapability->initialize(context)) {
        shutdown();
        return false;
    }

    connect(m_monitorCapability.get(), &SystemInfoMonitorCapability::snapshotUpdated, this,
        [this](const SystemInfoSnapshot& snapshot) {
            if (m_panelCapability) {
                m_panelCapability->updateSnapshot(snapshot);
            }
        });

    if (m_panelCapability) {
        m_panelCapability->updateSnapshot(m_monitorCapability->lastSnapshot());
    }

    return true;
}

void SystemInfoPlugin::start()
{
    if (m_monitorCapability) {
        m_monitorCapability->startMonitoring();
    }
}

void SystemInfoPlugin::stop()
{
    if (m_monitorCapability) {
        m_monitorCapability->stopMonitoring();
    }
}

void SystemInfoPlugin::shutdown()
{
    if (m_monitorCapability) {
        m_monitorCapability->stopMonitoring();
    }

    m_settingsCapability.reset();
    m_taskbarCapability.reset();
    m_panelCapability.reset();
    m_monitorCapability.reset();
    m_context = nullptr;
}

IMonitorCapability* SystemInfoPlugin::monitorCapability()
{
    return m_monitorCapability.get();
}

IPanelCapability* SystemInfoPlugin::panelCapability()
{
    return m_panelCapability.get();
}

ITaskbarCapability* SystemInfoPlugin::taskbarCapability()
{
    return m_taskbarCapability.get();
}

ISettingsCapability* SystemInfoPlugin::settingsCapability()
{
    return m_settingsCapability.get();
}

} // namespace Vitals
