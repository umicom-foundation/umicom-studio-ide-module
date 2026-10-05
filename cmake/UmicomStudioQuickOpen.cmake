# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise indexed-file Quick Open through the production editor toolbar.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-quick-open-test tests/test_quick_open_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-quick-open-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-quick-open-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-quick-open-test)
    umicom_apply_sanitizers(umicom-studio-quick-open-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-quick-open-test)
    endif()
    foreach(case button keyboard capture-key filter empty open dirty-existing stale-index refresh-index workspace-switch workspace-return retained-control repeated-open no-workspace)
        add_test(NAME studio.editor.quick_open.gtk4.${case} COMMAND umicom-studio-quick-open-test ${case})
        set_tests_properties(studio.editor.quick_open.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;editor;quick-open;gtk4;ownership;regression")
    endforeach()
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/QUICK_OPEN.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
