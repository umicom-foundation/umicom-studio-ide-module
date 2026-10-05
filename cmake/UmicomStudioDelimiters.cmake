# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise source delimiter navigation through the production editor toolbar.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-delimiter-test tests/test_delimiter_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-delimiter-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-delimiter-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-delimiter-test)
    umicom_apply_sanitizers(umicom-studio-delimiter-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-delimiter-test)
    endif()
    foreach(case match reverse contents pair c-string json-string read-only no-pair unicode folded-destination retained parent-close history)
        add_test(NAME studio.editor.delimiter.gtk4.${case} COMMAND umicom-studio-delimiter-test ${case})
        set_tests_properties(studio.editor.delimiter.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;delimiter;gtk4;ownership;regression")
    endforeach()
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/DELIMITERS.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
