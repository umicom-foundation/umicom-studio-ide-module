#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/UmicomStudioReadingPreview.cmake
#
# PURPOSE:
#   Compose Framework reading previews for every native workbench consumer.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------
include_guard(GLOBAL)
if(NOT TARGET Umicom::reading_gtk4)
    return()
endif()
# Discover all workbench consumers after this directory has created its tests.
# The product and acceptance executables must compile the same optional path.
function(umicom_studio_link_reading_preview)
    get_property(_consumers DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_consumer IN LISTS _consumers)
        get_target_property(_sources "${_consumer}" SOURCES)
        foreach(_source IN LISTS _sources)
            if(_source MATCHES "(^|/)workbench_window[.]c$")
                target_link_libraries("${_consumer}" PRIVATE Umicom::reading_gtk4)
                target_compile_definitions("${_consumer}" PRIVATE UMICOM_STUDIO_HAS_READING_GTK4=1)
                if(COMMAND umicom_target_qualification_inputs)
                    umicom_target_qualification_inputs("${_consumer}" FILES
                        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../src/gui/workbench/runtime/surfaces/live_preview.inc")
                endif()
                break()
            endif()
        endforeach()
    endforeach()
endfunction()
cmake_language(DEFER CALL umicom_studio_link_reading_preview)
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-reading-preview-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_live_preview_reading_gtk4.c"
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
    target_include_directories(umicom-studio-reading-preview-test PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
    get_target_property(_reading_framework_source umicom_developer SOURCE_DIR)
    target_include_directories(umicom-studio-reading-preview-test PRIVATE "${_reading_framework_source}/tests")
    target_link_libraries(umicom-studio-reading-preview-test PRIVATE
        Umicom::StudioCore Umicom::reading_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    set_target_properties(umicom-studio-reading-preview-test PROPERTIES
        C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reading-fixtures/empty/$<CONFIG>")
    umicom_apply_warnings(umicom-studio-reading-preview-test)
    umicom_apply_sanitizers(umicom-studio-reading-preview-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-reading-preview-test)
    endif()
    foreach(_case IN ITEMS markdown
            unicode
            code-font
            appearance
            source-preserved
            empty
            plain
            html
            retained-refresh
            close-summary
            close-buffer
            nested-refresh
            changed-source)
        add_test(NAME "studio.live_preview.reading.${_case}"
            COMMAND umicom-studio-reading-preview-test "${_case}")
        set_tests_properties("studio.live_preview.reading.${_case}" PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;preview;reading;gtk4;ownership;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners("studio.live_preview.reading.${_case}" TARGETS umicom-studio-reading-preview-test)
        endif()
    endforeach()
endif()
