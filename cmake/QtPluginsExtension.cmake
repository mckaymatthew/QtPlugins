# qtplugins_add_extension(<target> [sources...])
#
# Creates an extension (Qt plugin shared library) that links the SDK, is written to
# QTPLUGINS_EXTENSIONS_DIR in the build tree and installed to QTPLUGINS_INSTALL_EXTENSIONSDIR.
function(qtplugins_add_extension target)
    qt_add_plugin(${target} SHARED)
    target_sources(${target} PRIVATE ${ARGN})
    target_link_libraries(${target} PRIVATE QtPlugins::Sdk)
    # The generator expression stops multi-config generators (Visual Studio, Xcode) from
    # appending a per-configuration subdirectory.
    set_target_properties(${target} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "$<1:${QTPLUGINS_EXTENSIONS_DIR}>"
        RUNTIME_OUTPUT_DIRECTORY "$<1:${QTPLUGINS_EXTENSIONS_DIR}>"
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)
    install(TARGETS ${target}
        LIBRARY DESTINATION "${QTPLUGINS_INSTALL_EXTENSIONSDIR}"
        RUNTIME DESTINATION "${QTPLUGINS_INSTALL_EXTENSIONSDIR}")
endfunction()
