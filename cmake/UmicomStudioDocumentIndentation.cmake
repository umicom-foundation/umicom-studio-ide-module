# Umicom Studio IDE
# File: cmake/UmicomStudioDocumentIndentation.cmake
# PURPOSE: Qualify document-scoped editor policy through the native Studio workbench.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-document-indentation-test
        tests/test_document_indentation_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-document-indentation-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-document-indentation-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    set_target_properties(umicom-studio-document-indentation-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    umicom_apply_warnings(umicom-studio-document-indentation-test)
    umicom_apply_sanitizers(umicom-studio-document-indentation-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-document-indentation-test)
    endif()
    foreach(_indent_case IN ITEMS entry apply inherit inactive argument no-document readonly session-default)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/document-indentation/${_indent_case}")
        add_test(NAME studio.document.indentation.gtk4.${_indent_case} COMMAND umicom-studio-document-indentation-test ${_indent_case}
            "${CMAKE_CURRENT_BINARY_DIR}/qualification/document-indentation/${_indent_case}")
        set_tests_properties(studio.document.indentation.gtk4.${_indent_case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            ENVIRONMENT "G_DEBUG=fatal-warnings" LABELS "studio;editor;indentation;gtk4;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(studio.document.indentation.gtk4.${_indent_case} TARGETS umicom-studio-document-indentation-test)
        endif()
    endforeach()
endif()
install(FILES docs/DOCUMENT_INDENTATION.html DESTINATION share/umicom-studio/docs COMPONENT Applications)
unset(_indent_case)
