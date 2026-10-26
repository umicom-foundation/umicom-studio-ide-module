# Umicom Studio IDE
# File: cmake/UmicomStudioSavedTestReport.cmake
# PURPOSE: Register Studio integration coverage for Framework saved CTest report review.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umi-studio-saved-report-test tests/test_saved_ctest_report_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umi-studio-saved-report-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umi-studio-saved-report-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    set_target_properties(umi-studio-saved-report-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    umicom_apply_warnings(umi-studio-saved-report-test)
    umicom_apply_sanitizers(umi-studio-saved-report-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umi-studio-saved-report-test)
    endif()
    foreach(case IN ITEMS normal filter no-project retained cancel invalid-filter-preserves failed-replacement)
        set(directory "${CMAKE_CURRENT_BINARY_DIR}/qualification/saved_report/${case}")
        file(MAKE_DIRECTORY "${directory}")
        add_test(NAME studio.testing.saved_report.${case} COMMAND umi-studio-saved-report-test ${case} "${directory}")
        set_tests_properties(studio.testing.saved_report.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            ENVIRONMENT "G_DEBUG=fatal-warnings" LABELS "studio;testing;saved_report;gtk4;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(studio.testing.saved_report.${case} TARGETS umi-studio-saved-report-test)
        endif()
    endforeach()
endif()
install(FILES docs/SAVED-TEST-REPORTS.html DESTINATION share/umicom-studio/docs COMPONENT Applications)
unset(case)
unset(directory)
