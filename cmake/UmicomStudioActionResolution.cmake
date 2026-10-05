# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise the action-resolution form without starting a language server.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-action-resolution-test tests/test_action_resolution_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-action-resolution-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-action-resolution-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-action-resolution-test)
    umicom_apply_sanitizers(umicom-studio-action-resolution-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-action-resolution-test)
    endif()
    foreach(case open cancel retained empty parent-close)
        add_test(NAME studio.language.action_resolution.gtk4.${case} COMMAND umicom-studio-action-resolution-test ${case})
        set_tests_properties(studio.language.action_resolution.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;action-resolution;gtk4;ownership;regression")
    endforeach()
endif()


install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/RESOLVING_ACTIONS.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
