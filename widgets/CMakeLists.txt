add_library(VitalsWidgets STATIC
    CardWidget.h
    CardWidget.cpp
    LineChartWidget.h
    GaugeWidget.h
    MetricTableWidget.h
    MiniChartWidget.h
)
add_library(Vitals::Widgets ALIAS VitalsWidgets)

target_include_directories(VitalsWidgets
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(VitalsWidgets
    PUBLIC
        ${VITALS_QT_LIBS}
)

