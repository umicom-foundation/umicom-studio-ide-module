# Umicom Studio IDE
# File: cmake/UmicomStudioCheckpointMenus.cmake
#
# PURPOSE:
#   Register native checkpoint ownership, refusal and draft-preservation regressions.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-checkpoint-menu-test
        tests/test_checkpoint_menu_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-checkpoint-menu-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-checkpoint-menu-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-checkpoint-menu-test)
    umicom_apply_sanitizers(umicom-studio-checkpoint-menu-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-checkpoint-menu-test)
    endif()
    foreach(case no-storage-save no-storage-restore argument-save argument-restore
            busy-save busy-restore empty-restore menu-roundtrip direct-roundtrip
            compatibility-roundtrip transaction-save transaction-restore conflict
            retained-save retained-restore)
        add_test(NAME studio.checkpoint.menu.gtk4.${case}
            COMMAND umicom-studio-checkpoint-menu-test ${case})
        set_tests_properties(studio.checkpoint.menu.gtk4.${case}
            PROPERTIES TIMEOUT 90 SKIP_RETURN_CODE 77
            LABELS "studio;menu;gtk4;layout;storage;regression")
    endforeach()
endif()
