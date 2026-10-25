# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
# Test the same queued opening API used by the native chooser.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-open-files-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_open_files_gtk4.c"
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
    target_include_directories(umicom-studio-open-files-test PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
    target_link_libraries(umicom-studio-open-files-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-open-files-test)
    umicom_apply_sanitizers(umicom-studio-open-files-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-open-files-test)
    endif()
    # Retain the initial catalogue; drop cases share its real document assertions.
    if(FALSE)
    foreach(_studio_open_case IN ITEMS all mixed duplicate cancel busy captured retire empty limit dirty)
        endforeach()
    endif()
    foreach(_studio_open_case IN ITEMS all mixed duplicate cancel busy captured retire empty limit dirty drop drop-mixed drop-remote cancel-command results-command)
        add_test(NAME studio.documents.open_files.gtk4.${_studio_open_case}
            COMMAND umicom-studio-open-files-test ${_studio_open_case})
        set_tests_properties(studio.documents.open_files.gtk4.${_studio_open_case}
            PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 ENVIRONMENT "G_DEBUG=fatal-warnings"
            LABELS "studio;document;opening;gtk4;ownership;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(studio.documents.open_files.gtk4.${_studio_open_case}
                TARGETS umicom-studio-open-files-test)
        endif()
    endforeach()
    unset(_studio_open_case)
endif()
