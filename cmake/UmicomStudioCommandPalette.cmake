# Umicom Studio IDE
# File: cmake/UmicomStudioCommandPalette.cmake
#
# PURPOSE:
#   Register command palette publication, callback lifetime and draft-preservation regressions.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-command-palette-test
        tests/test_command_palette_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-command-palette-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-command-palette-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-command-palette-test)
    umicom_apply_sanitizers(umicom-studio-command-palette-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-command-palette-test)
    endif()
    foreach(case menu direct compatibility repeat argument retained-menu retained-choice retained-operation retained-search
            stale-choice stale-operation choose disabled observer-close observer-reenter observer-hide
            choice-close choice-reenter choice-show refresh-close refresh-reenter reset-close closed-palette)
        add_test(NAME studio.command_palette.gtk4.${case}
            COMMAND umicom-studio-command-palette-test ${case})
        set_tests_properties(studio.command_palette.gtk4.${case}
            PROPERTIES TIMEOUT 90 SKIP_RETURN_CODE 77
            LABELS "studio;menu;gtk4;commands;lifetime;regression")
    endforeach()
endif()
