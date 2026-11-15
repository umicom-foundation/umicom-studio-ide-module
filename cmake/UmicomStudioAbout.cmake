# Umicom Studio IDE
# File: cmake/UmicomStudioAbout.cmake
#
# PURPOSE:
#   Register native About menu, version and retained-owner regressions.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-about-test
        tests/test_about_gtk4.c src/gui/workbench/workbench_window.c src/gui/dialogs/about.c)
    target_include_directories(umicom-studio-about-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/dialogs/include")
    target_link_libraries(umicom-studio-about-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-about-test)
    umicom_apply_sanitizers(umicom-studio-about-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-about-test)
    endif()
    foreach(case menu direct repeat reopen argument retained-menu owner-close observer-close observer-reenter legacy legacy-alias)
        add_test(NAME studio.about.gtk4.${case}
            COMMAND umicom-studio-about-test ${case})
        set_tests_properties(studio.about.gtk4.${case}
            PROPERTIES TIMEOUT 90 SKIP_RETURN_CODE 77
            LABELS "studio;menu;gtk4;about;brand;lifetime;regression")
    endforeach()
endif()
