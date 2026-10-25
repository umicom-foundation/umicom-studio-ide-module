# Umicom Studio IDE
# File: cmake/UmicomStudioSelectionReplacement.cmake
# PURPOSE: Qualify captured selection replacement through the native Studio workbench.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-selection-replacement-test
        tests/test_selection_replace_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-selection-replacement-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-selection-replacement-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    set_target_properties(umicom-studio-selection-replacement-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    umicom_apply_warnings(umicom-studio-selection-replacement-test)
    umicom_apply_sanitizers(umicom-studio-selection-replacement-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-selection-replacement-test)
    endif()
    foreach(_indent_case IN ITEMS entry apply undo cancel readonly empty-selection argument no-document case-option inactive)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/selection-replacement/${_indent_case}")
        add_test(NAME studio.selection-replacement.gtk4.${_indent_case} COMMAND umicom-studio-selection-replacement-test ${_indent_case}
            "${CMAKE_CURRENT_BINARY_DIR}/qualification/selection-replacement/${_indent_case}")
        set_tests_properties(studio.selection-replacement.gtk4.${_indent_case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            ENVIRONMENT "G_DEBUG=fatal-warnings" LABELS "studio;editor;selection-replacement;gtk4;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(studio.selection-replacement.gtk4.${_indent_case} TARGETS umicom-studio-selection-replacement-test)
        endif()
    endforeach()
endif()
install(FILES docs/SELECTION-REPLACEMENT.html DESTINATION share/umicom-studio/docs COMPONENT Applications)
unset(_indent_case)
