# Umicom Studio IDE
# File: cmake/UmicomStudioEditorPreferenceStorage.cmake
# PURPOSE: Qualify background editor preferences against isolated storage and native lifecycle events.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-preference-storage-test
        tests/test_editor_preference_storage_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-preference-storage-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-preference-storage-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    # Redirect path choice in this executable only. The linked storage implementation remains real.
    target_compile_definitions(umicom-studio-preference-storage-test PRIVATE
        UmiDocumentIndentationStoragePath=FixtureEditorPreferenceStoragePath)
    set_target_properties(umicom-studio-preference-storage-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    umicom_apply_warnings(umicom-studio-preference-storage-test)
    umicom_apply_sanitizers(umicom-studio-preference-storage-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-preference-storage-test)
    endif()
    if(COMMAND umicom_target_qualification_inputs)
        umicom_target_qualification_inputs(umicom-studio-preference-storage-test FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/runtime/interaction/editor_preference_storage.inc")
    endif()
    foreach(_preference_storage_case IN ITEMS load save save-captured stale-load busy missing malformed retired
        default-policy startup button-save draft-not-applied close-after-save relative)
        set(_preference_storage_test "studio.editor.preference_storage.gtk4.${_preference_storage_case}")
        add_test(NAME ${_preference_storage_test} COMMAND umicom-studio-preference-storage-test ${_preference_storage_case})
        set_tests_properties(${_preference_storage_test} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            ENVIRONMENT "G_DEBUG=fatal-warnings" LABELS "studio;editor;preferences;storage;gtk4;ownership;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(${_preference_storage_test} TARGETS umicom-studio-preference-storage-test)
        endif()
    endforeach()
endif()
unset(_preference_storage_case)
unset(_preference_storage_test)
