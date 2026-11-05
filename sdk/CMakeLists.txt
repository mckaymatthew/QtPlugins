# Header-only SDK shared by the host and every extension.
add_library(qtplugins_sdk INTERFACE)
add_library(QtPlugins::Sdk ALIAS qtplugins_sdk)
target_include_directories(qtplugins_sdk INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(qtplugins_sdk INTERFACE Qt6::Core Qt6::Widgets)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
