# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise line editing through the production editor toolbar and document history.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-line-edits-test tests/test_line_edits_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-line-edits-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-line-edits-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-line-edits-test)
    umicom_apply_sanitizers(umicom-studio-line-edits-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-line-edits-test)
    endif()
    foreach(case duplicate delete move-up move-down join indent outdent comment trim selection read-only undo-redo retained-control no-neighbor)
        add_test(NAME studio.editor.line_edits.gtk4.${case} COMMAND umicom-studio-line-edits-test ${case})
        set_tests_properties(studio.editor.line_edits.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;editor;line-edit;gtk4;ownership;regression")
    endforeach()
endif()


install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/LINE_EDITING.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

include("${CMAKE_CURRENT_LIST_DIR}/UmicomStudioDocumentFormat.cmake")
