# qtplugins_add_extension(<target> [sources...])
#
# Creates an extension (Qt plugin shared library) that links the SDK and is written to
# QTPLUGINS_EXTENSIONS_DIR, where the host can load it.
function(qtplugins_add_extension target)
    qt_add_plugin(${target} SHARED)
    target_sources(${target} PRIVATE ${ARGN})
    target_link_libraries(${target} PRIVATE QtPlugins::Sdk)
    set_target_properties(${target} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${QTPLUGINS_EXTENSIONS_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${QTPLUGINS_EXTENSIONS_DIR}"
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)
endfunction()
