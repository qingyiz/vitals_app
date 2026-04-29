add_library(HelloPlugin MODULE
    HelloPlugin.h
    HelloPlugin.cpp
    hello_plugin.json
)

target_link_libraries(HelloPlugin
    PRIVATE
        Vitals::Sdk
        ${VITALS_QT_LIBS}
)

set_target_properties(HelloPlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
)

