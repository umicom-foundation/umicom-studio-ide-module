# Umicom Studio IDE
# File: cmake/UmicomStudioDocumentLanguage.cmake
# PURPOSE: Qualify explicit document language policy through the native Studio workbench.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-document-language-test
        tests/test_document_language_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-document-language-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-document-language-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    set_target_properties(umicom-studio-document-language-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    umicom_apply_warnings(umicom-studio-document-language-test)
    umicom_apply_sanitizers(umicom-studio-document-language-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-document-language-test)
    endif()
    foreach(_indent_case IN ITEMS entry apply automatic readonly inactive cancel argument no-document comment)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/document-language/${_indent_case}")
        add_test(NAME studio.document-language.gtk4.${_indent_case} COMMAND umicom-studio-document-language-test ${_indent_case}
            "${CMAKE_CURRENT_BINARY_DIR}/qualification/document-language/${_indent_case}")
        set_tests_properties(studio.document-language.gtk4.${_indent_case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            ENVIRONMENT "G_DEBUG=fatal-warnings" LABELS "studio;editor;document-language;gtk4;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(studio.document-language.gtk4.${_indent_case} TARGETS umicom-studio-document-language-test)
        endif()
    endforeach()
endif()
install(FILES docs/DOCUMENT-LANGUAGE.html DESTINATION share/umicom-studio/docs COMPONENT Applications)
unset(_indent_case)
