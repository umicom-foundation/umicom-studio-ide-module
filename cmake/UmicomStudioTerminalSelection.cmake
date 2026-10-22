# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
if(NOT BUILD_TESTING OR NOT TARGET umicom-studio-workspace-canvas-test)
    return()
endif()
# The production unity is included by this fixture to substitute only native
# folder dispatch. Model operations and window ownership remain production code.
add_executable(umicom-studio-terminal-selection-test tests/test_terminal_selection_gtk4.c)
target_include_directories(umicom-studio-terminal-selection-test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
target_link_libraries(umicom-studio-terminal-selection-test PRIVATE
    Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
set_target_properties(umicom-studio-terminal-selection-test PROPERTIES
    C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
umicom_apply_warnings(umicom-studio-terminal-selection-test)
umicom_apply_sanitizers(umicom-studio-terminal-selection-test)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umicom-studio-terminal-selection-test)
endif()
foreach(case session-seed draft-seed relative-seed empty-seed cancel inline-reply present-close create-close create-edit create-reenter title-close title-edit title-switch title-reenter seed-close seed-edit seed-switch late-close late-edit late-edit-return late-switch-return late-directory late-reparent reply-close render-folder-close render-output-close render-summary-close render-run-close render-stop-close render-switch render-reenter render-finished-session)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/terminal-selection-${case}")
    add_test(NAME studio.terminal_selection.gtk4.${case}
        COMMAND umicom-studio-terminal-selection-test "${case}"
        "${CMAKE_CURRENT_BINARY_DIR}/qualification/terminal-selection-${case}")
    set_tests_properties(studio.terminal_selection.gtk4.${case} PROPERTIES
        TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;terminal;gtk4;selection;ownership;regression")
endforeach()
