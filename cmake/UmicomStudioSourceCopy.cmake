# Umicom Studio IDE
# File: cmake/UmicomStudioSourceCopy.cmake
# PURPOSE: Exercise the native copy command against real document state.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-source-copy-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_save_copy_gtk4.c"
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
    target_include_directories(umicom-studio-source-copy-test PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
    target_link_libraries(umicom-studio-source-copy-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-source-copy-test)
    umicom_apply_sanitizers(umicom-studio-source-copy-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-source-copy-test)
    endif()
    foreach(_studio_copy_case IN ITEMS new existing same-source unicode readonly empty retired utf16)
        add_test(NAME studio.documents.copy.gtk4.${_studio_copy_case}
            COMMAND umicom-studio-source-copy-test ${_studio_copy_case})
        set_tests_properties(studio.documents.copy.gtk4.${_studio_copy_case}
            PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 ENVIRONMENT "G_DEBUG=fatal-warnings"
            LABELS "studio;document;copy;gtk4;ownership;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(studio.documents.copy.gtk4.${_studio_copy_case}
                TARGETS umicom-studio-source-copy-test)
        endif()
    endforeach()
    unset(_studio_copy_case)
endif()
