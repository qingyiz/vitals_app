add_library(SystemInfoPlugin MODULE
    SystemInfoPlugin.h
    SystemInfoPlugin.cpp
    monitor/SystemInfoMonitorCapability.h
    monitor/SystemInfoMonitorCapability.cpp
    panel/SystemInfoPanelCapability.h
    panel/SystemInfoPanelCapability.cpp
    taskbar/SystemInfoTaskbarCapability.h
    taskbar/SystemInfoTaskbarCapability.cpp
    settings/SystemInfoSettingsCapability.h
    settings/SystemInfoSettingsCapability.cpp
    SystemInfoPanelWidget.h
    SystemInfoPanelWidget.cpp
    ISystemInfoCollector.h
    SystemInfoCollectorFactory.h
    SystemInfoCollectorFactory.cpp
    systeminfo_plugin.json
)

if(APPLE)
    target_sources(SystemInfoPlugin
        PRIVATE
            platform/macos/MacSystemInfoCollector.h
            platform/macos/MacSystemInfoCollector.mm
    )
endif()

target_include_directories(SystemInfoPlugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SystemInfoPlugin
    PRIVATE
        Vitals::Sdk
        Vitals::Widgets
        ${VITALS_QT_LIBS}
)

if(APPLE)
    target_link_libraries(SystemInfoPlugin
        PRIVATE
            "-framework Foundation"
            "-framework Metal"
    )
endif()

set_target_properties(SystemInfoPlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
)
