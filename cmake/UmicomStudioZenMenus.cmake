# Umicom Studio IDE
# File: cmake/UmicomStudioZenMenus.cmake
#
# PURPOSE:
#   Register native Zen selection and draft-preservation regressions.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-zen-menu-test
        tests/test_zen_menu_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-zen-menu-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-zen-menu-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-zen-menu-test)
    umicom_apply_sanitizers(umicom-studio-zen-menu-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-zen-menu-test)
    endif()
    foreach(case menu direct custom direct-zen intervening busy argument retained)
        add_test(NAME studio.zen.menu.gtk4.${case}
            COMMAND umicom-studio-zen-menu-test ${case})
        set_tests_properties(studio.zen.menu.gtk4.${case}
            PROPERTIES TIMEOUT 90 SKIP_RETURN_CODE 77
            LABELS "studio;zen;menu;gtk4;layout;regression")
    endforeach()
endif()
