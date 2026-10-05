# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise the syntax-selection action without starting a language server.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-selection-ranges-test tests/test_selection_ranges_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-selection-ranges-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-selection-ranges-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-selection-ranges-test)
    umicom_apply_sanitizers(umicom-studio-selection-ranges-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-selection-ranges-test)
    endif()
    foreach(case open cancel retained empty parent-close)
        add_test(NAME studio.language.selection_ranges.gtk4.${case} COMMAND umicom-studio-selection-ranges-test ${case})
        set_tests_properties(studio.language.selection_ranges.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;selection-ranges;gtk4;ownership;regression")
    endforeach()
endif()


install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/SYNTAX_SELECTION.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
