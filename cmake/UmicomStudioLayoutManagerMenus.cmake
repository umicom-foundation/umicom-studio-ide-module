# Umicom Studio IDE
# File: cmake/UmicomStudioLayoutManagerMenus.cmake
#
# PURPOSE:
#   Register layout browser ownership, selection and draft-preservation regressions.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-layout-manager-menu-test
        tests/test_layout_manager_menu_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-layout-manager-menu-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-layout-manager-menu-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-layout-manager-menu-test)
    umicom_apply_sanitizers(umicom-studio-layout-manager-menu-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-layout-manager-menu-test)
    endif()
    foreach(case menu direct compatibility repeat argument retained-menu retained-choice
            choose busy-choice observer-close observer-reenter choice-close choice-reenter closed-browser)
        add_test(NAME studio.layout_manager.menu.gtk4.${case}
            COMMAND umicom-studio-layout-manager-menu-test ${case})
        set_tests_properties(studio.layout_manager.menu.gtk4.${case}
            PROPERTIES TIMEOUT 90 SKIP_RETURN_CODE 77
            LABELS "studio;menu;gtk4;layout;lifetime;regression")
    endforeach()
endif()
