# Umicom Studio IDE
# File: cmake/UmicomStudioDocumentationOwnership.cmake
#
# PURPOSE:
#   Register native Documentation navigation, search and retained-control regressions.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-documentation-ownership-test
        tests/test_documentation_ownership_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-documentation-ownership-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-documentation-ownership-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-documentation-ownership-test)
    umicom_apply_sanitizers(umicom-studio-documentation-ownership-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-documentation-ownership-test)
    endif()
    foreach(case open activate file-uri back forward reload search search-empty search-missing relative-no-project invalid-uri remote retained-open retained-back retained-forward retained-reload retained-search address-close address-reenter buffer-close buffer-reenter label-close label-reenter buffer-replace selection-close selection-reenter)
        add_test(NAME studio.documentation_ownership.gtk4.${case}
            COMMAND umicom-studio-documentation-ownership-test ${case})
        set_tests_properties(studio.documentation_ownership.gtk4.${case}
            PROPERTIES TIMEOUT 90 SKIP_RETURN_CODE 77
            LABELS "studio;documentation;gtk4;search;lifetime;regression")
    endforeach()
endif()
