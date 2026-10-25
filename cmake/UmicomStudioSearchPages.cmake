# Umicom Studio IDE
# File: cmake/UmicomStudioSearchPages.cmake
# PURPOSE: Qualify native search paging through the production workbench.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
include_guard(GLOBAL)
# Install the guide even when native regression targets are disabled.
install(FILES "${CMAKE_CURRENT_LIST_DIR}/../docs/SEARCH-RESULT-PAGES.html"
    DESTINATION "share/umicom-studio/docs" COMPONENT Studio)
if(NOT BUILD_TESTING OR NOT TARGET Umicom::ui_gtk4 OR NOT TARGET Umicom::StudioCore)
    return()
endif()
add_executable(umi-studio-search-pages-test
    "${CMAKE_CURRENT_LIST_DIR}/../tests/test_search_pages_gtk4.c"
    "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
target_include_directories(umi-studio-search-pages-test PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
target_link_libraries(umi-studio-search-pages-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4
    PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
set_target_properties(umi-studio-search-pages-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
umicom_apply_warnings(umi-studio-search-pages-test)
umicom_apply_sanitizers(umi-studio-search-pages-test)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umi-studio-search-pages-test)
endif()
if(COMMAND umicom_target_qualification_inputs)
    umicom_target_qualification_inputs(umi-studio-search-pages-test FILES
        "${CMAKE_CURRENT_LIST_DIR}/../tests/search_pages_fixture.h"
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/runtime/interaction/search_pages.inc"
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/runtime/surfaces/search.inc")
endif()
foreach(case IN ITEMS normal next previous last first unchanged old-row new-request stale-index filter-query empty retired summary-close scope-close range-close rows-close refresh-reentry start-reentry workspace-during-publication invalid-retired)
    set(test "studio.file_search.pages.${case}")
    set(directory "${CMAKE_CURRENT_BINARY_DIR}/qualification/search-pages/${case}")
    file(MAKE_DIRECTORY "${directory}")
    add_test(NAME ${test} COMMAND umi-studio-search-pages-test ${case})
    set_tests_properties(${test} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
        WORKING_DIRECTORY "${directory}" ENVIRONMENT "G_DEBUG=fatal-warnings"
        LABELS "studio;gtk4;search;page;regression")
    if(COMMAND umicom_test_impact_owners)
        umicom_test_impact_owners(${test} TARGETS umi-studio-search-pages-test)
    endif()
endforeach()
# Retain the former test-gated installation for review. The unconditional rule above replaces it.
if(FALSE)
install(FILES "${CMAKE_CURRENT_LIST_DIR}/../docs/SEARCH-RESULT-PAGES.html"
    DESTINATION "share/umicom-studio/docs" COMPONENT Studio)
endif()
