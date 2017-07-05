set(KRITA_TOUCHDOCKER_SOURCES 
    TouchDockerPlugin.cpp 
    TouchDockerDock.cpp 
)

ki18n_wrap_ui(KRITA_TOUCHDOCKER_SOURCES
    WdgTouchDocker.ui
)
add_library(kritatouchdocker MODULE ${KRITA_TOUCHDOCKER_SOURCES})
target_link_libraries(kritatouchdocker kritaui)
install(TARGETS kritatouchdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
