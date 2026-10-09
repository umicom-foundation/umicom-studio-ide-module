# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
# Native history actions use isolated SQLite storage and never submit a build.
include_guard(GLOBAL)
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-job-history-native-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_build_job_history_gtk4.c"
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
    target_include_directories(umicom-studio-job-history-native-test PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
    target_link_libraries(umicom-studio-job-history-native-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-job-history-native-test)
    umicom_apply_sanitizers(umicom-studio-job-history-native-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-job-history-native-test)
    endif()
    # Keep the former registration list for review as a non-executing CMake comment.
    #[=[
    foreach(case reopen invalid detach prune retained-open retained-refresh retained-prune retained-detach)
    ]=]
    foreach(case identity reopen invalid detach prune retained-open retained-refresh retained-prune retained-detach)
        add_test(NAME studio.job_history.gtk4.${case} COMMAND umicom-studio-job-history-native-test "${case}")
        set_tests_properties(studio.job_history.gtk4.${case} PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;job-history;gtk4;persistence;ownership;regression")
    endforeach()
endif()
install(FILES "${CMAKE_CURRENT_LIST_DIR}/../docs/LOCAL_JOB_HISTORY.html"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
