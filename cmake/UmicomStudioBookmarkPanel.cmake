# Umicom Studio IDE
# File: cmake/UmicomStudioBookmarkPanel.cmake
# PURPOSE: Qualify native bookmark navigation through the production workbench.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
# Install the guide even when native regression targets are disabled.
install(FILES "${CMAKE_CURRENT_LIST_DIR}/../docs/SESSION-BOOKMARKS.html"
    DESTINATION "share/umicom-studio/docs" COMPONENT Studio)
if(NOT BUILD_TESTING OR NOT TARGET Umicom::ui_gtk4 OR NOT TARGET Umicom::StudioCore)
    return()
endif()
add_executable(umi-studio-bookmark-panel-test
    "${CMAKE_CURRENT_LIST_DIR}/../tests/test_bookmark_panel_gtk4.c"
    "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
target_include_directories(umi-studio-bookmark-panel-test PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
target_link_libraries(umi-studio-bookmark-panel-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4
    PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
set_target_properties(umi-studio-bookmark-panel-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
umicom_apply_warnings(umi-studio-bookmark-panel-test)
umicom_apply_sanitizers(umi-studio-bookmark-panel-test)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umi-studio-bookmark-panel-test)
endif()
if(COMMAND umicom_target_qualification_inputs)
    umicom_target_qualification_inputs(umi-studio-bookmark-panel-test FILES
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/runtime/surfaces/bookmarks.inc"
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/runtime/runtime_types.inc")
endif()
foreach(case IN ITEMS visit hidden-editor remove closed retained refresh catalogue-bound)
    set(test "studio.bookmark_panel.${case}")
    set(directory "${CMAKE_CURRENT_BINARY_DIR}/qualification/bookmark-panel/${case}")
    file(MAKE_DIRECTORY "${directory}")
    add_test(NAME ${test} COMMAND umi-studio-bookmark-panel-test ${case})
    set_tests_properties(${test} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
        WORKING_DIRECTORY "${directory}" ENVIRONMENT "G_DEBUG=fatal-warnings"
        LABELS "studio;gtk4;bookmarks;regression")
    if(COMMAND umicom_test_impact_owners)
        umicom_test_impact_owners(${test} TARGETS umi-studio-bookmark-panel-test)
    endif()
endforeach()
