add_library(VitalsSdk INTERFACE)
add_library(Vitals::Sdk ALIAS VitalsSdk)

target_include_directories(VitalsSdk
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(VitalsSdk
    INTERFACE
        ${VITALS_QT_LIBS}
)

