# Umicom Studio IDE
# File: cmake/UmicomStudioDebuggerMenus.cmake
#
# PURPOSE:
#   Register debugger menu reveal and owner-lifetime regressions.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-debug-menu-test
        tests/test_debug_menu_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-debug-menu-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-debug-menu-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-debug-menu-test)
    umicom_apply_sanitizers(umicom-studio-debug-menu-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-debug-menu-test)
    endif()
    foreach(intent attach breakpoints)
        foreach(case menu direct repeat argument retained observer-close observer-reenter)
            add_test(NAME studio.debug.menu.gtk4.${intent}.${case}
                COMMAND umicom-studio-debug-menu-test debug.${intent} ${case})
            set_tests_properties(studio.debug.menu.gtk4.${intent}.${case}
                PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
                LABELS "studio;debug;menu;gtk4;ownership;regression")
        endforeach()
    endforeach()
endif()

# Target identity must survive notebook observers, not only a live window.
if(BUILD_TESTING AND TARGET umicom-studio-debug-menu-test)
    foreach(case observer-redirect observer-remove observer-hide observer-reorder)
        add_test(NAME studio.debug.menu.gtk4.breakpoints.${case}
            COMMAND umicom-studio-debug-menu-test debug.breakpoints ${case})
        set_tests_properties(studio.debug.menu.gtk4.breakpoints.${case}
            PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            LABELS "studio;debug;menu;gtk4;ownership;regression")
    endforeach()
endif()
