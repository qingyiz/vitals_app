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

if(WIN32)
    target_sources(MemoryMonitorPlugin
        PRIVATE
            platform/windows/WindowsMemoryCollector.h
            platform/windows/WindowsMemoryCollector.cpp
    )
endif()

if(UNIX AND NOT APPLE)
    target_sources(MemoryMonitorPlugin
        PRIVATE
            platform/linux/LinuxMemoryCollector.h
            platform/linux/LinuxMemoryCollector.cpp
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
    LIBRARY_OUTPUT_DIRECTORY "${VITALS_PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${VITALS_PLUGIN_OUTPUT_DIR}"
)

if(MSVC)
    target_compile_options(MemoryMonitorPlugin PRIVATE /utf-8)
endif()

if(WIN32 AND CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${config}" config_upper)
        set_target_properties(MemoryMonitorPlugin PROPERTIES
            "LIBRARY_OUTPUT_DIRECTORY_${config_upper}" "${VITALS_OUTPUT_DIR}/${config}/plugins"
            "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${VITALS_OUTPUT_DIR}/${config}/plugins"
        )
    endforeach()
endif()
