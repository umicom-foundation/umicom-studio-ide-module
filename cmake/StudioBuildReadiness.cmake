#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: applications/studio/cmake/StudioBuildReadiness.cmake
# PURPOSE:
#   Build the actual native Studio targets together before a larger rebuild.
#   Never let a headless configuration report native readiness as success.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
#-----------------------------------------------------------------------------
if(BUILD_TESTING)
    add_executable(umicom-studio-document-contract-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_workbench_document_contracts.c")
    target_link_libraries(umicom-studio-document-contract-test PRIVATE Umicom::document)
    umicom_apply_warnings(umicom-studio-document-contract-test)
    umicom_apply_sanitizers(umicom-studio-document-contract-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-document-contract-test)
    endif()
    add_test(NAME studio.build_readiness.document_contracts
        COMMAND umicom-studio-document-contract-test)
    set_tests_properties(studio.build_readiness.document_contracts PROPERTIES
        TIMEOUT 30 LABELS "studio;build;public-contract;regression")
endif()

# Original native prerequisite condition retained for the migration record:
# if(TARGET umicom-studio-ide AND TARGET umicom-studio-workbench-demo AND
#         TARGET umicom-studio-workspace-canvas-test AND TARGET umicom_ui_gtk4 AND
#         TARGET umicom-studio-document-contract-test)
if(TARGET umicom-studio-ide AND TARGET umicom-studio-workbench-demo AND
        TARGET umicom-studio-workspace-canvas-test AND TARGET umicom_ui_gtk4 AND
        TARGET umicom-studio-document-contract-test AND TARGET umicom-save-link-readiness)
    # Previous condition also required the three native consumers and contracts.
    # The additional Framework-owned save-link target now verifies the example
    # and GTK test consumers which previously failed later in the all build.
    # Former final dependency line, retained for the migration record:
    # umicom-studio-workspace-canvas-test umicom-studio-document-contract-test
    # Reuse the production targets. In particular, all three compile the real
    # workbench_window.c, not a shortened stand-in that can miss private includes.
    add_custom_target(umicom-studio-build-readiness
        DEPENDS umicom_document umicom-studio-ide umicom-studio-workbench-demo
                umicom-studio-workspace-canvas-test umicom-studio-document-contract-test
        COMMENT "Compile Studio, its demo and native canvas test with document contracts")
    # The save-link checks live in Framework; express the cross-directory edge
    # as a target dependency rather than treating its name as an output file.
    add_dependencies(umicom-studio-build-readiness umicom-save-link-readiness)
else()
    # The named command is deliberately available but fails here. A green
    # headless build cannot establish that the native translation unit compiled.
    # Original explanation retained; the new message also identifies the link gate:
    # "Studio native readiness requires GTK4, Studio native targets and BUILD_TESTING=ON."
    add_custom_target(umicom-studio-build-readiness
        COMMAND "${CMAKE_COMMAND}" -E echo
            "Studio native readiness requires GTK4, the Framework save-link target, native Studio targets and BUILD_TESTING=ON."
        COMMAND "${CMAKE_COMMAND}" -E false
        VERBATIM)
endif()

# The existing readiness command also links the real close fixture when native
# GTK targets are present. The headless refusal above remains unchanged.
if(TARGET umicom-document-closing-test)
    add_dependencies(umicom-studio-build-readiness umicom-document-closing-test)
endif()
if(TARGET umicom-document-closing-gtk4-test)
    add_dependencies(umicom-studio-build-readiness umicom-document-closing-gtk4-test)
endif()
