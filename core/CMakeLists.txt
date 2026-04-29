add_library(VitalsCore STATIC
    plugin/PluginManager.h
    plugin/PluginManager.cpp
    metric/MetricCenter.h
    metric/MetricCenter.cpp
    metric/MetricRegistry.h
    metric/MetricRegistry.cpp
    config/ConfigManager.h
    config/ConfigManager.cpp
    event/EventBus.h
    event/EventBus.cpp
    log/Logger.h
    log/Logger.cpp
)
add_library(Vitals::Core ALIAS VitalsCore)

target_include_directories(VitalsCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(VitalsCore
    PUBLIC
        Vitals::Sdk
        ${VITALS_QT_LIBS}
)

