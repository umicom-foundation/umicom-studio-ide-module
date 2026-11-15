# Umicom Studio IDE
# File: cmake/UmicomStudioWelcomeNavigation.cmake
#
# PURPOSE:
#   Register explicit Welcome navigation and retained-document regressions.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-welcome-navigation-test
        tests/test_welcome_navigation_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-welcome-navigation-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-welcome-navigation-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-welcome-navigation-test)
    umicom_apply_sanitizers(umicom-studio-welcome-navigation-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-welcome-navigation-test)
    endif()
    foreach(case menu direct repeat refresh return return-button new-document argument return-argument
            retained-menu retained-return observer-close observer-reenter return-close)
        add_test(NAME studio.welcome.navigation.gtk4.${case}
            COMMAND umicom-studio-welcome-navigation-test ${case})
        set_tests_properties(studio.welcome.navigation.gtk4.${case}
            PROPERTIES TIMEOUT 90 SKIP_RETURN_CODE 77
            LABELS "studio;menu;gtk4;welcome;lifetime;regression")
    endforeach()
endif()
