# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise selection restoration through Studio native typing and shared document history.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-typing-history-test tests/test_typing_history_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-typing-history-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-typing-history-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-typing-history-test)
    umicom_apply_sanitizers(umicom-studio-typing-history-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-typing-history-test)
    endif()
    foreach(case selection reverse unicode multiline caret nested retained-buffer)
        add_test(NAME studio.editor.typing_history.gtk4.${case} COMMAND umicom-studio-typing-history-test ${case})
        set_tests_properties(studio.editor.typing_history.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;editor;typing-history;gtk4;ownership;regression")
    endforeach()
endif()


install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/UNDO_REDO.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
