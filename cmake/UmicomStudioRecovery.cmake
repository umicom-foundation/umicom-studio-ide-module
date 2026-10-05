# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise private recovery through the production editor toolbar and shared document owner.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-recovery-test tests/test_recovery_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-recovery-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-recovery-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-recovery-test)
    umicom_apply_sanitizers(umicom-studio-recovery-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-recovery-test)
    endif()
    foreach(case open empty-list capture restore unicode retained-control parent-close automatic-enable automatic-pause)
        add_test(NAME studio.editor.recovery.gtk4.${case} COMMAND umicom-studio-recovery-test ${case})
        set_tests_properties(studio.editor.recovery.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;editor;recovery;gtk4;ownership;regression")
    endforeach()
endif()


install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/DRAFT_RECOVERY.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
