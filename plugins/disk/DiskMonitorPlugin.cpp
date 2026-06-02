#include "DiskMonitorPlugin.h"

#include "monitor/DiskMonitorCapability.h"
#include "panel/DiskPanelCapability.h"
#include "settings/DiskSettingsCapability.h"
#include "taskbar/DiskTaskbarCapability.h"

namespace Vitals {

DiskMonitorPlugin::DiskMonitorPlugin(QObject* parent)
    : QObject(parent)
{
}

DiskMonitorPlugin::~DiskMonitorPlugin() = default;

PluginMetaInfo DiskMonitorPlugin::metaInfo() const
{
    return {
        QStringLiteral("com.vitals.disk"),
        QStringLiteral("Disk Monitor"),
        QStringLiteral("macOS mounted disk capacity monitor with selectable internal and external drives."),
        QStringLiteral("0.1.0"),
        QStringLiteral("Vitals"),
        QStringLiteral("monitor"),
        {QStringLiteral("macos")},
        QStringLiteral("0.1.0"),
        true
    };
}

bool DiskMonitorPlugin::initialize(IAppContext* context)
{
    m_context = context;

    m_monitorCapability = std::make_unique<DiskMonitorCapability>();
    m_panelCapability = std::make_unique<DiskPanelCapability>(m_monitorCapability.get(), context);
    m_taskbarCapability = std::make_unique<DiskTaskbarCapability>(m_monitorCapability.get(), context);
    m_settingsCapability = std::make_unique<DiskSettingsCapability>(m_monitorCapability.get(), context);

    if (!m_monitorCapability->initialize(context)) {
        shutdown();
        return false;
    }

    connect(m_monitorCapability.get(), &DiskMonitorCapability::snapshotUpdated, this,
        [this](const DiskSnapshot& snapshot) {
            if (m_panelCapability) {
                m_panelCapability->updateSnapshot(snapshot);
            }
            if (m_settingsCapability) {
                m_settingsCapability->updateSnapshot(snapshot);
            }
        });

    m_monitorCapability->refreshNow();
    if (m_panelCapability) {
        m_panelCapability->updateSnapshot(m_monitorCapability->lastSnapshot());
    }
    if (m_settingsCapability) {
        m_settingsCapability->updateSnapshot(m_monitorCapability->lastSnapshot());
    }

    return true;
}

void DiskMonitorPlugin::start()
{
    if (m_monitorCapability) {
        m_monitorCapability->startMonitoring();
    }
}

void DiskMonitorPlugin::stop()
{
    if (m_monitorCapability) {
        m_monitorCapability->stopMonitoring();
    }
}

void DiskMonitorPlugin::shutdown()
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

IMonitorCapability* DiskMonitorPlugin::monitorCapability()
{
    return m_monitorCapability.get();
}

IPanelCapability* DiskMonitorPlugin::panelCapability()
{
    return m_panelCapability.get();
}

ITaskbarCapability* DiskMonitorPlugin::taskbarCapability()
{
    return m_taskbarCapability.get();
}

ISettingsCapability* DiskMonitorPlugin::settingsCapability()
{
    return m_settingsCapability.get();
}

} // namespace Vitals
