#include "NetworkMonitorPlugin.h"

#include "monitor/NetworkMonitorCapability.h"
#include "panel/NetworkPanelCapability.h"
#include "settings/NetworkSettingsCapability.h"
#include "taskbar/NetworkTaskbarCapability.h"

namespace Vitals {

NetworkMonitorPlugin::NetworkMonitorPlugin(QObject* parent)
    : QObject(parent)
{
}

NetworkMonitorPlugin::~NetworkMonitorPlugin() = default;

PluginMetaInfo NetworkMonitorPlugin::metaInfo() const
{
    return {
        QStringLiteral("com.vitals.network"),
        QStringLiteral("Network Monitor"),
        QStringLiteral("Live network throughput and traffic counters for macOS."),
        QStringLiteral("0.1.0"),
        QStringLiteral("Vitals"),
        QStringLiteral("monitor"),
        {QStringLiteral("macos")},
        QStringLiteral("0.1.0"),
        true
    };
}

bool NetworkMonitorPlugin::initialize(IAppContext* context)
{
    m_context = context;

    m_monitorCapability = std::make_unique<NetworkMonitorCapability>();
    m_panelCapability = std::make_unique<NetworkPanelCapability>();
    m_taskbarCapability = std::make_unique<NetworkTaskbarCapability>(m_monitorCapability.get());
    m_settingsCapability = std::make_unique<NetworkSettingsCapability>();

    if (!m_monitorCapability->initialize(context)) {
        shutdown();
        return false;
    }

    connect(m_monitorCapability.get(), &NetworkMonitorCapability::snapshotUpdated, this,
        [this](const NetworkSnapshot& snapshot) {
            if (m_panelCapability) {
                m_panelCapability->updateSnapshot(snapshot);
            }
        });

    if (m_panelCapability) {
        m_panelCapability->updateSnapshot(m_monitorCapability->lastSnapshot());
    }

    return true;
}

void NetworkMonitorPlugin::start()
{
    if (m_monitorCapability) {
        m_monitorCapability->startMonitoring();
    }
}

void NetworkMonitorPlugin::stop()
{
    if (m_monitorCapability) {
        m_monitorCapability->stopMonitoring();
    }
}

void NetworkMonitorPlugin::shutdown()
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

IMonitorCapability* NetworkMonitorPlugin::monitorCapability()
{
    return m_monitorCapability.get();
}

IPanelCapability* NetworkMonitorPlugin::panelCapability()
{
    return m_panelCapability.get();
}

ITaskbarCapability* NetworkMonitorPlugin::taskbarCapability()
{
    return m_taskbarCapability.get();
}

ISettingsCapability* NetworkMonitorPlugin::settingsCapability()
{
    return m_settingsCapability.get();
}

} // namespace Vitals
