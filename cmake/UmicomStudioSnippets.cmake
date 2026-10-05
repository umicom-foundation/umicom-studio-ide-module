# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise local template composition through the production editor toolbar.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-snippet-test tests/test_snippet_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-snippet-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-snippet-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-snippet-test)
    umicom_apply_sanitizers(umicom-studio-snippet-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-snippet-test)
    endif()
    foreach(case open cancel retained empty parent-close insert undo)
        add_test(NAME studio.editor.snippet.gtk4.${case} COMMAND umicom-studio-snippet-test ${case})
        set_tests_properties(studio.editor.snippet.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;snippet;gtk4;ownership;regression")
    endforeach()
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/SNIPPETS.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
