# Umicom Studio IDE
# File: cmake/UmicomStudioNavigationMenus.cmake
#
# PURPOSE:
#   Register real navigation menu, toolbar and retained-control regressions.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-navigation-menu-test
        tests/test_navigation_menu_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-navigation-menu-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-navigation-menu-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-navigation-menu-test)
    umicom_apply_sanitizers(umicom-studio-navigation-menu-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-navigation-menu-test)
    endif()
    foreach(intent symbol definition references)
        foreach(case open cancel retained busy argument toolbar read-only no-document parent-close)
            add_test(NAME studio.language.navigation_menu.gtk4.${intent}.${case}
                COMMAND umicom-studio-navigation-menu-test ${intent} ${case})
            set_tests_properties(studio.language.navigation_menu.gtk4.${intent}.${case}
                PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
                LABELS "studio;language;navigation;menu;gtk4;ownership;regression")
        endforeach()
    endforeach()
endif()
