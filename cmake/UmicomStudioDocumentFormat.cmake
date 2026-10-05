# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise document format through the production editor toolbar and document history.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-document-format-test tests/test_document_format_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-document-format-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-document-format-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-document-format-test)
    umicom_apply_sanitizers(umicom-studio-document-format-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-document-format-test)
    endif()
    foreach(case lf crlf utf8 utf8-bom utf16-le utf16-be read-only undo-redo retained-control)
        add_test(NAME studio.editor.document_format.gtk4.${case} COMMAND umicom-studio-document-format-test ${case})
        set_tests_properties(studio.editor.document_format.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;editor;line-edit;gtk4;ownership;regression")
    endforeach()
endif()


install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/FILE_FORMATS.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

include("${CMAKE_CURRENT_LIST_DIR}/UmicomStudioDecodedSearch.cmake")
