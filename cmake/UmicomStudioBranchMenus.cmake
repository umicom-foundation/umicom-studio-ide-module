# Umicom Studio IDE
# File: cmake/UmicomStudioBranchMenus.cmake
#
# PURPOSE:
#   Register native branch navigation without repository side effects.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    # The fixture includes the production host to exercise its private row factory.
    # Retain separate translation-unit wiring for comparison, without duplicate symbols.
    if(FALSE)
    add_executable(umicom-studio-branch-menu-test
        tests/test_branch_menu_gtk4.c src/gui/workbench/workbench_window.c)
    endif()
    add_executable(umicom-studio-branch-menu-test tests/test_branch_menu_gtk4.c)
    target_include_directories(umicom-studio-branch-menu-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-branch-menu-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-branch-menu-test)
    umicom_apply_sanitizers(umicom-studio-branch-menu-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-branch-menu-test)
    endif()
    foreach(case menu direct repeat argument retained no-workspace closed-workspace
            observer-close observer-project observer-reenter observer-redirect observer-remove)
        add_test(NAME studio.source_control.branches.gtk4.${case}
            COMMAND umicom-studio-branch-menu-test ${case})
        set_tests_properties(studio.source_control.branches.gtk4.${case}
            PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            LABELS "studio;source-control;menu;gtk4;ownership;regression")
    endforeach()
endif()

if(BUILD_TESTING AND TARGET umicom-studio-branch-menu-test)
    foreach(case valid current disabled orphan replaced project-close project-revision
            retired callback-close registry-denied)
        add_test(NAME studio.source_control.checkout.gtk4.${case}
            COMMAND umicom-studio-branch-menu-test checkout-${case})
        set_tests_properties(studio.source_control.checkout.gtk4.${case}
            PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            LABELS "studio;source-control;checkout;gtk4;ownership;regression")
    endforeach()
endif()
