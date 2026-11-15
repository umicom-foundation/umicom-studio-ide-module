# Umicom Studio IDE
# File: cmake/UmicomStudioShortcutReference.cmake
#
# PURPOSE:
#   Register native shortcut reference and retained-document regressions.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-shortcut-reference-test
        tests/test_shortcut_reference_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-shortcut-reference-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-shortcut-reference-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-shortcut-reference-test)
    umicom_apply_sanitizers(umicom-studio-shortcut-reference-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-shortcut-reference-test)
    endif()
    foreach(case menu direct repeat argument retained-menu buffer-close label-close buffer-reenter
            label-reenter buffer-replace location-close location-reenter reopen)
        add_test(NAME studio.shortcut_reference.gtk4.${case}
            COMMAND umicom-studio-shortcut-reference-test ${case})
        set_tests_properties(studio.shortcut_reference.gtk4.${case}
            PROPERTIES TIMEOUT 90 SKIP_RETURN_CODE 77
            LABELS "studio;menu;gtk4;shortcuts;lifetime;regression")
    endforeach()
endif()
