# Umicom Studio IDE
# File: cmake/UmicomStudioLanguageComments.cmake
# PURPOSE: Qualify language-aware source commenting through the native Studio workbench.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-language-comments-test
        tests/test_language_comment_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-language-comments-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-language-comments-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    set_target_properties(umicom-studio-language-comments-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    umicom_apply_warnings(umicom-studio-language-comments-test)
    umicom_apply_sanitizers(umicom-studio-language-comments-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-language-comments-test)
    endif()
    foreach(_indent_case IN ITEMS c python sql cmake readonly unknown argument undo)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/language-comments/${_indent_case}")
        add_test(NAME studio.language-comments.gtk4.${_indent_case} COMMAND umicom-studio-language-comments-test ${_indent_case}
            "${CMAKE_CURRENT_BINARY_DIR}/qualification/language-comments/${_indent_case}")
        set_tests_properties(studio.language-comments.gtk4.${_indent_case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            ENVIRONMENT "G_DEBUG=fatal-warnings" LABELS "studio;editor;language-comments;gtk4;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(studio.language-comments.gtk4.${_indent_case} TARGETS umicom-studio-language-comments-test)
        endif()
    endforeach()
endif()
install(FILES docs/LANGUAGE-COMMENTS.html DESTINATION share/umicom-studio/docs COMPONENT Applications)
unset(_indent_case)
