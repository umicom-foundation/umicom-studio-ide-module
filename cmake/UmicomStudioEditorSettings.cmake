# Umicom Studio IDE
# File: cmake/UmicomStudioEditorSettings.cmake
# PURPOSE: Qualify stored and refreshed indentation preferences through the shared document service.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
if(BUILD_TESTING)
    add_executable(umicom-studio-editor-settings-test tests/test_editor_settings.c)
    target_link_libraries(umicom-studio-editor-settings-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-editor-settings-test)
    umicom_apply_sanitizers(umicom-studio-editor-settings-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-editor-settings-test)
    endif()
    foreach(_editor_settings_case IN ITEMS defaults startup spaces tabs refresh invalid arguments)
        set(_editor_settings_test "studio.editor.settings.${_editor_settings_case}")
        add_test(NAME ${_editor_settings_test} COMMAND umicom-studio-editor-settings-test ${_editor_settings_case})
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/editor-settings/${_editor_settings_case}")
        set_tests_properties(${_editor_settings_test} PROPERTIES TIMEOUT 60
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/editor-settings/${_editor_settings_case}"
            ENVIRONMENT "UMICOM_STUDIO_DATA_PATH="
            LABELS "studio;editor;settings;indentation;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(${_editor_settings_test} TARGETS umicom-studio-editor-settings-test)
        endif()
    endforeach()
    unset(_editor_settings_case)
    unset(_editor_settings_test)
endif()
