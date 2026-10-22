# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
if(NOT BUILD_TESTING OR NOT TARGET umicom-studio-workspace-canvas-test)
    return()
endif()
# Observe native row parenting through the production workbench. No compiler
# processes, replacement list renderer or remote services are started.
add_executable(umicom-studio-problems-publication-test tests/test_problems_publication_gtk4.c)
target_include_directories(umicom-studio-problems-publication-test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
target_link_libraries(umicom-studio-problems-publication-test PRIVATE
    Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
set_target_properties(umicom-studio-problems-publication-test PROPERTIES
    C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
umicom_apply_warnings(umicom-studio-problems-publication-test)
umicom_apply_sanitizers(umicom-studio-problems-publication-test)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umicom-studio-problems-publication-test)
endif()
foreach(case normal search severity sort resolved empty selection filter-selection sort-selection unchanged bounded remove-close append-close summary-close selection-close filter-during-remove filter-return-during-remove model-during-remove reenter-during-remove filter-during-append model-during-append retained-row retained-controls clear clear-close source-dirty)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/problems-publication-${case}")
    add_test(NAME studio.problems_publication.gtk4.${case}
        COMMAND umicom-studio-problems-publication-test "${case}"
        "${CMAKE_CURRENT_BINARY_DIR}/qualification/problems-publication-${case}")
    set_tests_properties(studio.problems_publication.gtk4.${case} PROPERTIES
        TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;diagnostics;gtk4;publication;ownership;regression")
endforeach()
