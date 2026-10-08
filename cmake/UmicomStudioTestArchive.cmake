# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
# Studio binds lifecycle and presentation; Framework owns archive storage.
include_guard(GLOBAL)
target_link_libraries(umicom_studio_core PUBLIC Umicom::test_archive)
if(BUILD_TESTING)
    add_executable(umicom-studio-test-archive-service-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_test_archive.c")
    # Reuse the Framework fixture's exclusive native temporary-directory helper.
    get_target_property(_archive_framework_source umicom_testing SOURCE_DIR)
    target_include_directories(umicom-studio-test-archive-service-test PRIVATE
        "${_archive_framework_source}/tests/build_log")
    target_link_libraries(umicom-studio-test-archive-service-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-test-archive-service-test)
    umicom_apply_sanitizers(umicom-studio-test-archive-service-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-test-archive-service-test)
    endif()
    foreach(case invalid no-run reopen detach wrong-thread retain-output remove
        compare-read compare-cancel compare-busy compare-missing compare-retry compare-remove)
        add_test(NAME studio.test_archive.service.${case}
            COMMAND umicom-studio-test-archive-service-test "${case}")
        set_tests_properties(studio.test_archive.service.${case} PROPERTIES
            TIMEOUT 30 SKIP_RETURN_CODE 77 LABELS "studio;testing;archive;persistence;regression")
    endforeach()
endif()
install(FILES "${CMAKE_CURRENT_LIST_DIR}/../docs/PRIVATE_TEST_HISTORY.html"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Retained controls exercise the existing weak-window action binding.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-test-archive-native-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_test_archive_gtk4.c"
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
    target_include_directories(umicom-studio-test-archive-native-test PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
    target_link_libraries(umicom-studio-test-archive-native-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-test-archive-native-test)
    umicom_apply_sanitizers(umicom-studio-test-archive-native-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-test-archive-native-test)
    endif()
    foreach(case invalid save-read remove retained-open retained-refresh retained-save retained-stop
            retained-read retained-remove retained-detach compare compare-page-invalid
            retained-compare retained-compare.stop retained-compare.page)
        add_test(NAME studio.test_archive.gtk4.${case}
            COMMAND umicom-studio-test-archive-native-test "${case}")
        set_tests_properties(studio.test_archive.gtk4.${case} PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;testing;archive;gtk4;ownership;regression")
    endforeach()
endif()
