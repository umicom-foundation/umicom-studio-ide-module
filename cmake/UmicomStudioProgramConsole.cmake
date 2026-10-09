# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
install(FILES "${CMAKE_CURRENT_LIST_DIR}/../docs/PROGRAM_CONSOLE.html"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/umicom-studio/docs")
if(NOT BUILD_TESTING OR NOT TARGET umicom-studio-workspace-canvas-test OR
   NOT TARGET umicom-program-console-producer)
    return()
endif()
if(NOT WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    return()
endif()
add_executable(umicom-studio-program-console-test
    "${CMAKE_CURRENT_LIST_DIR}/../tests/test_program_console_gtk4.c"
    "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
target_include_directories(umicom-studio-program-console-test PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
target_link_libraries(umicom-studio-program-console-test PRIVATE
    Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
set_target_properties(umicom-studio-program-console-test PROPERTIES
    C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
add_dependencies(umicom-studio-program-console-test umicom-program-console-producer)
umicom_apply_warnings(umicom-studio-program-console-test)
umicom_apply_sanitizers(umicom-studio-program-console-test)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umicom-studio-program-console-test)
endif()
foreach(case input retained revoke project-close parent-close denied)
    add_test(NAME studio.program_console.gtk4.${case}
        COMMAND umicom-studio-program-console-test "${case}" "$<TARGET_FILE:umicom-program-console-producer>")
    set_tests_properties(studio.program_console.gtk4.${case} PROPERTIES
        TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;console;gtk4;ownership;acceptance")
endforeach()
