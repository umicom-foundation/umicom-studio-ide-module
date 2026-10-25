# Umicom Studio IDE
# File: cmake/UmicomStudioEditorConfig.cmake
# PURPOSE: Qualify the shared inspector through actual Studio command routing.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-editor-config-native-test
        tests/test_editor_config_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-editor-config-native-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-editor-config-native-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    set_target_properties(umicom-studio-editor-config-native-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    umicom_apply_warnings(umicom-studio-editor-config-native-test)
    umicom_apply_sanitizers(umicom-studio-editor-config-native-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-editor-config-native-test)
    endif()
    foreach(_config_case IN ITEMS entry untrusted no-project argument active-file independent switch-project)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/editor-config/${_config_case}")
        add_test(NAME studio.editor.config.gtk4.${_config_case} COMMAND umicom-studio-editor-config-native-test ${_config_case}
            "${CMAKE_CURRENT_BINARY_DIR}/qualification/editor-config/${_config_case}")
        set_tests_properties(studio.editor.config.gtk4.${_config_case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            ENVIRONMENT "G_DEBUG=fatal-warnings"
            LABELS "studio;editor;configuration;gtk4;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(studio.editor.config.gtk4.${_config_case} TARGETS umicom-studio-editor-config-native-test)
        endif()
    endforeach()
endif()
install(FILES docs/PROJECT_EDITOR_CONFIG.html DESTINATION share/umicom-studio/docs COMPONENT Applications)
unset(_config_case)
