#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/UmicomStudioCodeGuardNative.cmake
#
# PURPOSE:
# Register native CodeGuard action, output and callback ownership qualification.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------

if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    function(umicom_studio_codeguard_native_target target)
        add_executable(${target} tests/test_codeguard_native_gtk4.c
            src/gui/workbench/workbench_window.c)
        target_include_directories(${target} PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
        target_link_libraries(${target} PRIVATE Umicom::StudioCore Umicom::ui_gtk4
            PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
        target_compile_features(${target} PRIVATE c_std_23)
        umicom_apply_warnings(${target})
        umicom_apply_sanitizers(${target})
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(${target})
        endif()
        if(COMMAND umicom_target_qualification_inputs)
            umicom_target_qualification_inputs(${target} FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_test_state.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/runtime/interaction/codeguard.inc"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/runtime/runtime_unity.inc"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/runtime/runtime_helpers.inc"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/runtime/runtime_refresh.inc"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/runtime/surfaces/output.inc")
        endif()
    endfunction()
    umicom_studio_codeguard_native_target(umicom-studio-codeguard-native-test)
    foreach(_case IN ITEMS menu direct output-button untrusted closed argument repeat busy cancel-menu cancel-direct cancel-closed cancel-idle retained-scan retained-cancel observer-close observer-switch observer-reenter observer-collapse output-close output-switch output-reenter paused-context)
        add_test(NAME "studio.codeguard_native.${_case}"
            COMMAND umicom-studio-codeguard-native-test "${_case}")
        set_tests_properties("studio.codeguard_native.${_case}" PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;codeguard;gtk4;ownership")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners("studio.codeguard_native.${_case}"
                TARGETS umicom-studio-codeguard-native-test)
        endif()
    endforeach()
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT APPLE AND NOT MSVC)
        umicom_studio_codeguard_native_target(umicom-studio-codeguard-native-fault-test)
        target_compile_definitions(umicom-studio-codeguard-native-fault-test PRIVATE
            UMICOM_STUDIO_CODEGUARD_NATIVE_FAULTS=1)
        target_link_options(umicom-studio-codeguard-native-fault-test PRIVATE
            -Wl,--wrap=UmiStudioCodeGuardPoll)
        foreach(_case IN ITEMS poll-failure poll-recovery poll-context-failure)
            add_test(NAME "studio.codeguard_native_fault.${_case}"
                COMMAND umicom-studio-codeguard-native-fault-test "${_case}")
            set_tests_properties("studio.codeguard_native_fault.${_case}" PROPERTIES
                TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;codeguard;gtk4;ownership;fault")
            if(COMMAND umicom_test_impact_owners)
                umicom_test_impact_owners("studio.codeguard_native_fault.${_case}"
                    TARGETS umicom-studio-codeguard-native-fault-test)
            endif()
        endforeach()
    endif()
endif()
