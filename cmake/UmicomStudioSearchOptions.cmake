# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise literal search policy through the production editor toolbar.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-search-options-test tests/test_search_options_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-search-options-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-search-options-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-search-options-test)
    umicom_apply_sanitizers(umicom-studio-search-options-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-search-options-test)
    endif()
    foreach(case defaults sensitive insensitive smart-uppercase whole-word previous single-boundary single-insensitive all-review all-owned session-review session-owned set-review set-owned retained-options read-only-find)
        add_test(NAME studio.editor.search_options.gtk4.${case} COMMAND umicom-studio-search-options-test ${case})
        set_tests_properties(studio.editor.search_options.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;editor;search-options;gtk4;ownership;regression")
    endforeach()
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/FIND_AND_REPLACE.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
