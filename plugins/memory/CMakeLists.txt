add_library(MemoryMonitorPlugin MODULE
    MemoryMonitorPlugin.h
    MemoryMonitorPlugin.cpp
    monitor/MemoryMonitorCapability.h
    monitor/MemoryMonitorCapability.cpp
    panel/MemoryPanelCapability.h
    panel/MemoryPanelCapability.cpp
    taskbar/MemoryTaskbarCapability.h
    taskbar/MemoryTaskbarCapability.cpp
    MemoryPanelWidget.h
    MemoryPanelWidget.cpp
    IMemoryCollector.h
    MemoryCollectorFactory.h
    MemoryCollectorFactory.cpp
    memory_plugin.json
)

if(APPLE)
    target_sources(MemoryMonitorPlugin
        PRIVATE
            platform/macos/MacMemoryCollector.h
            platform/macos/MacMemoryCollector.mm
    )
endif()

target_include_directories(MemoryMonitorPlugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(MemoryMonitorPlugin
    PRIVATE
        Vitals::Sdk
        Vitals::Widgets
        ${VITALS_QT_LIBS}
)

set_target_properties(MemoryMonitorPlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
)
