find_package(Qt6 REQUIRED COMPONENTS Test)

# Broken/foreign libraries the ExtensionManager must reject, built into their own directory.
set(FIXTURE_DIR "${CMAKE_CURRENT_BINARY_DIR}/fixture-extensions")
foreach(fixture wrongiid badapi)
    qt_add_plugin(fixture_${fixture} SHARED)
    target_sources(fixture_${fixture} PRIVATE fixtures/${fixture}.cpp)
    target_link_libraries(fixture_${fixture} PRIVATE QtPlugins::Sdk)
    set_target_properties(fixture_${fixture} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${FIXTURE_DIR}")
endforeach()
add_library(fixture_notaplugin SHARED fixtures/notaplugin.cpp)
set_target_properties(fixture_notaplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${FIXTURE_DIR}")
file(WRITE "${FIXTURE_DIR}/README.txt" "Not a library; must be ignored.\n")

qt_add_executable(tst_extensionmanager tst_extensionmanager.cpp)
target_link_libraries(tst_extensionmanager PRIVATE qtplugins_extensionmanager Qt6::Widgets Qt6::Test)
target_compile_definitions(tst_extensionmanager PRIVATE
    EXTENSIONS_DIR="${QTPLUGINS_EXTENSIONS_DIR}"
    FIXTURE_DIR="${FIXTURE_DIR}")
add_dependencies(tst_extensionmanager hello fixture_wrongiid fixture_badapi fixture_notaplugin)
add_test(NAME tst_extensionmanager COMMAND tst_extensionmanager)
set_tests_properties(tst_extensionmanager PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
