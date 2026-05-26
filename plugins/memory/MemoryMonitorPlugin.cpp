#include "MemoryMonitorPlugin.h"

#include "monitor/MemoryMonitorCapability.h"
#include "panel/MemoryPanelCapability.h"
#include "taskbar/MemoryTaskbarCapability.h"

namespace Vitals {

MemoryMonitorPlugin::MemoryMonitorPlugin(QObject* parent)
    : QObject(parent)
{
}

MemoryMonitorPlugin::~MemoryMonitorPlugin() = default;

PluginMetaInfo MemoryMonitorPlugin::metaInfo() const
{
    return {
        QStringLiteral("com.vitals.memory"),
        QStringLiteral("Memory Monitor"),
        QStringLiteral("Live physical memory usage monitor plugin for macOS."),
        QStringLiteral("0.1.0"),
        QStringLiteral("Vitals"),
        QStringLiteral("monitor"),
        {QStringLiteral("macos")},
        QStringLiteral("0.1.0"),
        true
    };
}

bool MemoryMonitorPlugin::initialize(IAppContext* context)
{
    m_context = context;

    m_monitorCapability = std::make_unique<MemoryMonitorCapability>();
    m_panelCapability = std::make_unique<MemoryPanelCapability>();
    m_taskbarCapability = std::make_unique<MemoryTaskbarCapability>(m_monitorCapability.get());

    if (!m_monitorCapability->initialize(context)) {
        shutdown();
        return false;
    }

    connect(m_monitorCapability.get(), &MemoryMonitorCapability::snapshotUpdated, this,
        [this](const MemorySnapshot& snapshot) {
            if (m_panelCapability) {
                m_panelCapability->updateSnapshot(snapshot);
            }
        });

    if (m_panelCapability) {
        m_panelCapability->updateSnapshot(m_monitorCapability->lastSnapshot());
    }

    return true;
}

void MemoryMonitorPlugin::start()
{
    if (m_monitorCapability) {
        m_monitorCapability->startMonitoring();
    }
}

void MemoryMonitorPlugin::stop()
{
    if (m_monitorCapability) {
        m_monitorCapability->stopMonitoring();
    }
}

void MemoryMonitorPlugin::shutdown()
{
    if (m_monitorCapability) {
        m_monitorCapability->stopMonitoring();
    }

    m_taskbarCapability.reset();
    m_panelCapability.reset();
    m_monitorCapability.reset();
    m_context = nullptr;
}

IMonitorCapability* MemoryMonitorPlugin::monitorCapability()
{
    return m_monitorCapability.get();
}

IPanelCapability* MemoryMonitorPlugin::panelCapability()
{
    return m_panelCapability.get();
}

ITaskbarCapability* MemoryMonitorPlugin::taskbarCapability()
{
    return m_taskbarCapability.get();
}

} // namespace Vitals
