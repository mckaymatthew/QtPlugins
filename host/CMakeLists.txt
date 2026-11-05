# ExtensionManager is a separate library only so tests can link it; in a real host just add
# ExtensionManager.{h,cpp} to the application's sources.
add_library(qtplugins_extensionmanager STATIC ExtensionManager.h ExtensionManager.cpp)
target_include_directories(qtplugins_extensionmanager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(qtplugins_extensionmanager PUBLIC QtPlugins::Sdk Qt6::Core)

qt_add_executable(qtplugins-host main.cpp MainWindow.h MainWindow.cpp)
target_link_libraries(qtplugins-host PRIVATE qtplugins_extensionmanager Qt6::Widgets)
set_target_properties(qtplugins-host PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE)

install(TARGETS qtplugins-host
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
