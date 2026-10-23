# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
if(NOT BUILD_TESTING OR NOT TARGET umicom-studio-workspace-canvas-test)
    return()
endif()
# Exercise navigation over the real diagnostic model and GTK controls.
add_executable(umicom-studio-problems-paging-test tests/test_problems_paging_gtk4.c)
target_include_directories(umicom-studio-problems-paging-test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
target_link_libraries(umicom-studio-problems-paging-test PRIVATE
    Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
set_target_properties(umicom-studio-problems-paging-test PROPERTIES
    C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
umicom_apply_warnings(umicom-studio-problems-paging-test)
umicom_apply_sanitizers(umicom-studio-problems-paging-test)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umicom-studio-problems-paging-test)
endif()
foreach(case first next last previous filter sort severity resolved shrink clear empty unchanged stale-next retained-row retained-controls source range-close previous-close next-close range-query range-return range-reenter invalid-bytes)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/problems-paging-${case}")
    add_test(NAME studio.problems_paging.gtk4.${case}
        COMMAND umicom-studio-problems-paging-test "${case}"
        "${CMAKE_CURRENT_BINARY_DIR}/qualification/problems-paging-${case}")
    set_tests_properties(studio.problems_paging.gtk4.${case} PROPERTIES
        TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;diagnostics;gtk4;navigation;ownership;regression")
endforeach()
