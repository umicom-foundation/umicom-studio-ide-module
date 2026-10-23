# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
if(NOT BUILD_TESTING OR NOT TARGET umicom-studio-workspace-canvas-test)
    return()
endif()
# Run the production command-entry path with a fixture-only acceptance handler.
# No executable is started; native draft and ownership callbacks remain real.
add_executable(umicom-studio-terminal-draft-test tests/test_terminal_draft_gtk4.c)
target_include_directories(umicom-studio-terminal-draft-test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
target_link_libraries(umicom-studio-terminal-draft-test PRIVATE
    Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
set_target_properties(umicom-studio-terminal-draft-test PROPERTIES
    C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
umicom_apply_warnings(umicom-studio-terminal-draft-test)
umicom_apply_sanitizers(umicom-studio-terminal-draft-test)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umicom-studio-terminal-draft-test)
endif()
foreach(case accepted rejected edit edit-return dispatch-close clear-close clear-edit clear-reenter reenter retry entry-activate empty)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/terminal-draft-${case}")
    add_test(NAME studio.terminal_draft.gtk4.${case}
        COMMAND umicom-studio-terminal-draft-test "${case}"
        "${CMAKE_CURRENT_BINARY_DIR}/qualification/terminal-draft-${case}")
    set_tests_properties(studio.terminal_draft.gtk4.${case} PROPERTIES
        TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;terminal;draft;gtk4;ownership;regression")
endforeach()
