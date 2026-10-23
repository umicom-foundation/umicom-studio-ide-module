# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
if(NOT BUILD_TESTING OR NOT TARGET umicom-studio-workspace-canvas-test)
    return()
endif()
# Exercise the production page controls and guarded rows with isolated discovered records.
# No build, test process or remote provider is launched by this fixture.
add_executable(umicom-studio-explorer-paging-test tests/test_explorer_paging_gtk4.c)
target_include_directories(umicom-studio-explorer-paging-test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
target_link_libraries(umicom-studio-explorer-paging-test PRIVATE
    Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
set_target_properties(umicom-studio-explorer-paging-test PROPERTIES
    C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
umicom_apply_warnings(umicom-studio-explorer-paging-test)
umicom_apply_sanitizers(umicom-studio-explorer-paging-test)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umicom-studio-explorer-paging-test)
endif()
foreach(case first next last previous filter sort shrink empty unchanged stale-next stale-row retained-row retained-search range-close summary-close query-reenter query-away-back rows-close rows-reenter rows-query invalid-bytes select run debug run-reselect debug-reselect source failed-only disabled capacity-message jump-last jump-first jump-stale jump-filter jump-shrink catalogue-navigation jump-retained jump-close)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/explorer-paging-${case}")
    add_test(NAME studio.explorer_paging.gtk4.${case}
        COMMAND umicom-studio-explorer-paging-test "${case}"
        "${CMAKE_CURRENT_BINARY_DIR}/qualification/explorer-paging-${case}")
    set_tests_properties(studio.explorer_paging.gtk4.${case} PROPERTIES
        TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;test-platform;paging;gtk4;ownership;regression")
endforeach()

# The large-catalogue fixture owns a full discovery registry. Keep it isolated
# from other memory-heavy GUI cases while exercising the real native controls.
set_tests_properties(studio.explorer_paging.gtk4.catalogue-navigation PROPERTIES
    TIMEOUT 120 RUN_SERIAL TRUE)
