add_library(DiskMonitorPlugin MODULE
    DiskMonitorPlugin.h
    DiskMonitorPlugin.cpp
    monitor/DiskMonitorCapability.h
    monitor/DiskMonitorCapability.cpp
    panel/DiskPanelCapability.h
    panel/DiskPanelCapability.cpp
    taskbar/DiskTaskbarCapability.h
    taskbar/DiskTaskbarCapability.cpp
    settings/DiskSettingsCapability.h
    settings/DiskSettingsCapability.cpp
    DiskPanelWidget.h
    DiskPanelWidget.cpp
    IDiskCollector.h
    DiskCollectorFactory.h
    DiskCollectorFactory.cpp
    disk_plugin.json
)

if(APPLE)
    target_sources(DiskMonitorPlugin
        PRIVATE
            platform/macos/MacDiskCollector.h
            platform/macos/MacDiskCollector.cpp
    )
endif()

if(WIN32)
    target_sources(DiskMonitorPlugin
        PRIVATE
            platform/windows/WindowsDiskCollector.h
            platform/windows/WindowsDiskCollector.cpp
    )
endif()

target_include_directories(DiskMonitorPlugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DiskMonitorPlugin
    PRIVATE
        Vitals::Sdk
        Vitals::Widgets
        ${VITALS_QT_LIBS}
)

set_target_properties(DiskMonitorPlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${VITALS_PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${VITALS_PLUGIN_OUTPUT_DIR}"
)

if(MSVC)
    target_compile_options(DiskMonitorPlugin PRIVATE /utf-8)
endif()

if(WIN32 AND CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${config}" config_upper)
        set_target_properties(DiskMonitorPlugin PROPERTIES
            "LIBRARY_OUTPUT_DIRECTORY_${config_upper}" "${VITALS_OUTPUT_DIR}/${config}/plugins"
            "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${VITALS_OUTPUT_DIR}/${config}/plugins"
        )
    endforeach()
endif()
