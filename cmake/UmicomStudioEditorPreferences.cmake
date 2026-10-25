# Umicom Studio IDE
# File: cmake/UmicomStudioEditorPreferences.cmake
# PURPOSE: Integrate editor preference publication and native form regression coverage.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
target_sources(umicom_studio_core PRIVATE src/app/editor_preferences.c)
if(BUILD_TESTING)
    add_executable(umicom-studio-editor-preferences-test tests/test_editor_preferences.c)
    target_link_libraries(umicom-studio-editor-preferences-test PRIVATE Umicom::StudioCore)
    set(_preferences_targets umicom-studio-editor-preferences-test)
    foreach(_preferences_case IN ITEMS read spaces tabs stale-width stale-spaces invalid arguments repeat unrelated session-only draft)
        set(_preferences_test "studio.editor.preferences.${_preferences_case}")
        add_test(NAME ${_preferences_test} COMMAND umicom-studio-editor-preferences-test ${_preferences_case})
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/editor-preferences/${_preferences_case}")
        set_tests_properties(${_preferences_test} PROPERTIES TIMEOUT 60
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/editor-preferences/${_preferences_case}"
            ENVIRONMENT "UMICOM_STUDIO_DATA_PATH=" LABELS "studio;editor;preferences;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(${_preferences_test} TARGETS umicom-studio-editor-preferences-test)
        endif()
    endforeach()
    if(TARGET umicom-studio-workspace-canvas-test)
        add_executable(umicom-studio-editor-preferences-native-test
            tests/test_editor_preferences_gtk4.c src/gui/workbench/workbench_window.c)
        target_include_directories(umicom-studio-editor-preferences-native-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
        target_link_libraries(umicom-studio-editor-preferences-native-test PRIVATE
            Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
        list(APPEND _preferences_targets umicom-studio-editor-preferences-native-test)
        foreach(_preferences_case IN ITEMS apply tabs cancel invalid stale repeat retained-button retired-owner close-notification owner-during-open reentrant)
            set(_preferences_test "studio.editor.preferences.gtk4.${_preferences_case}")
            add_test(NAME ${_preferences_test} COMMAND umicom-studio-editor-preferences-native-test ${_preferences_case})
            set_tests_properties(${_preferences_test} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
                ENVIRONMENT "G_DEBUG=fatal-warnings" LABELS "studio;editor;preferences;gtk4;ownership;regression")
            if(COMMAND umicom_test_impact_owners)
                umicom_test_impact_owners(${_preferences_test} TARGETS umicom-studio-editor-preferences-native-test)
            endif()
        endforeach()
    endif()
    foreach(_preferences_target IN LISTS _preferences_targets)
        set_target_properties(${_preferences_target} PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
        umicom_apply_warnings(${_preferences_target})
        umicom_apply_sanitizers(${_preferences_target})
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(${_preferences_target})
        endif()
    endforeach()
endif()
install(FILES docs/EDITOR_PREFERENCES.html DESTINATION share/umicom-studio/docs COMPONENT Applications)
unset(_preferences_case)
unset(_preferences_test)
unset(_preferences_target)
unset(_preferences_targets)
