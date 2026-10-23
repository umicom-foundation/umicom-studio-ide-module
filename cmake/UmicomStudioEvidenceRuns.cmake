# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
if(NOT BUILD_TESTING OR NOT TARGET umicom-studio-workspace-canvas-test)
    return()
endif()
# Exercise the production run picker and evidence panes with isolated retained records.
# No build, test process or remote provider is launched by this fixture.
add_executable(umicom-studio-evidence-runs-test tests/test_evidence_runs_gtk4.c)
target_include_directories(umicom-studio-evidence-runs-test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
target_link_libraries(umicom-studio-evidence-runs-test PRIVATE
    Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
set_target_properties(umicom-studio-evidence-runs-test PROPERTIES
    C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
umicom_apply_warnings(umicom-studio-evidence-runs-test)
umicom_apply_sanitizers(umicom-studio-evidence-runs-test)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umicom-studio-evidence-runs-test)
endif()
foreach(case all exact output-only unspecified missing-run back-all test-change unchanged retained-row retained-picker model-close summary-close model-reselect refresh-reenter invalid-bytes csv source stale-picker source-reselect)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/evidence-runs-${case}")
    add_test(NAME studio.evidence_runs.gtk4.${case}
        COMMAND umicom-studio-evidence-runs-test "${case}"
        "${CMAKE_CURRENT_BINARY_DIR}/qualification/evidence-runs-${case}")
    set_tests_properties(studio.evidence_runs.gtk4.${case} PROPERTIES
        TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;test-platform;evidence;gtk4;ownership;regression")
endforeach()
