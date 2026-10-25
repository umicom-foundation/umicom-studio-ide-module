# Umicom Studio IDE
# File: cmake/UmicomStudioDebugBacktrace.cmake
# PURPOSE: Register Studio integration coverage for Framework call-stack review.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umi-studio-backtrace-test tests/test_debug_backtrace_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umi-studio-backtrace-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umi-studio-backtrace-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    set_target_properties(umi-studio-backtrace-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    umicom_apply_warnings(umi-studio-backtrace-test)
    umicom_apply_sanitizers(umi-studio-backtrace-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umi-studio-backtrace-test)
    endif()
    foreach(case IN ITEMS normal empty running denied hidden retained owner-close)
        set(directory "${CMAKE_CURRENT_BINARY_DIR}/qualification/backtrace/${case}")
        file(MAKE_DIRECTORY "${directory}")
        add_test(NAME studio.debug.backtrace.${case} COMMAND umi-studio-backtrace-test ${case} "${directory}")
        set_tests_properties(studio.debug.backtrace.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            ENVIRONMENT "G_DEBUG=fatal-warnings" LABELS "studio;debugger;backtrace;gtk4;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(studio.debug.backtrace.${case} TARGETS umi-studio-backtrace-test)
        endif()
    endforeach()
endif()
install(FILES docs/CAPTURED-CALL-STACKS.html DESTINATION share/umicom-studio/docs COMPONENT Applications)
unset(case)
unset(directory)
