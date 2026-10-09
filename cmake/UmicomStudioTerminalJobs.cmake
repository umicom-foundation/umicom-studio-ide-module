# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
# Exercise the real product controls against an isolated Framework test producer.
include_guard(GLOBAL)
include(GNUInstallDirs)
install(FILES "${CMAKE_CURRENT_LIST_DIR}/../docs/TERMINAL_COMMANDS.html"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/umicom-studio/docs")

if(NOT BUILD_TESTING OR NOT TARGET umicom-studio-workspace-canvas-test OR
   NOT TARGET umicom-terminal-output-producer)
    return()
endif()
add_executable(umicom-studio-terminal-jobs-test
    "${CMAKE_CURRENT_LIST_DIR}/../tests/test_terminal_jobs_gtk4.c"
    "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
target_include_directories(umicom-studio-terminal-jobs-test PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
target_link_libraries(umicom-studio-terminal-jobs-test PRIVATE
    Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
set_target_properties(umicom-studio-terminal-jobs-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
add_dependencies(umicom-studio-terminal-jobs-test umicom-terminal-output-producer)
umicom_apply_warnings(umicom-studio-terminal-jobs-test)
umicom_apply_sanitizers(umicom-studio-terminal-jobs-test)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umicom-studio-terminal-jobs-test)
endif()
foreach(case complete stop revoke denied selection retained directory)
    add_test(NAME studio.terminal_jobs.gtk4.${case}
        COMMAND umicom-studio-terminal-jobs-test "${case}" "$<TARGET_FILE:umicom-terminal-output-producer>")
    set_tests_properties(studio.terminal_jobs.gtk4.${case} PROPERTIES
        TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;terminal;gtk4;ownership;regression")
endforeach()
