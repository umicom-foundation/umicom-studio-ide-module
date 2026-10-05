# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise manual source folding through the production editor toolbar.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-folding-test tests/test_folding_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-folding-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-folding-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-folding-test)
    umicom_apply_sanitizers(umicom-studio-folding-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-folding-test)
    endif()
    foreach(case fold reveal source-history clean dirty caret edit refresh second-document retained parent-close no-selection single-line)
        add_test(NAME studio.editor.folding.gtk4.${case} COMMAND umicom-studio-folding-test ${case})
        set_tests_properties(studio.editor.folding.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;folding;gtk4;ownership;regression")
    endforeach()
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/FOLDING.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
