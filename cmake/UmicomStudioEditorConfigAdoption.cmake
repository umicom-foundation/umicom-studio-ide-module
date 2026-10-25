# Umicom Studio IDE
# File: cmake/UmicomStudioEditorConfigAdoption.cmake
# PURPOSE: Qualify explicit project configuration adoption through Studio's existing workbench.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-editor-config-adoption-test
        tests/test_editor_config_adoption_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-editor-config-adoption-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-editor-config-adoption-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    set_target_properties(umicom-studio-editor-config-adoption-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    umicom_apply_warnings(umicom-studio-editor-config-adoption-test)
    umicom_apply_sanitizers(umicom-studio-editor-config-adoption-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-editor-config-adoption-test)
    endif()
    foreach(_indent_case IN ITEMS entry apply untrusted no-project no-document untitled outside-project argument inactive)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/editor-config-adoption/${_indent_case}")
        add_test(NAME studio.editor.config.adoption.gtk4.${_indent_case} COMMAND umicom-studio-editor-config-adoption-test ${_indent_case}
            "${CMAKE_CURRENT_BINARY_DIR}/qualification/editor-config-adoption/${_indent_case}")
        set_tests_properties(studio.editor.config.adoption.gtk4.${_indent_case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            ENVIRONMENT "G_DEBUG=fatal-warnings" LABELS "studio;editor;indentation;gtk4;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(studio.editor.config.adoption.gtk4.${_indent_case} TARGETS umicom-studio-editor-config-adoption-test)
        endif()
    endforeach()
endif()
install(FILES docs/EDITOR_CONFIG_ADOPTION.html DESTINATION share/umicom-studio/docs COMPONENT Applications)
unset(_indent_case)
