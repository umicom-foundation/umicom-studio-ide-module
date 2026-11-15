#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/UmicomStudioCodeGuardCli.cmake
#
# PURPOSE:
# Register command-line report and native argument boundary qualification.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------

if(BUILD_TESTING)
    function(umicom_studio_codeguard_cli_target target)
        add_executable(${target} tests/test_codeguard_cli.c)
        target_link_libraries(${target} PRIVATE Umicom::StudioCore PkgConfig::GLIB)
        target_compile_features(${target} PRIVATE c_std_23)
        umicom_apply_warnings(${target})
        umicom_apply_sanitizers(${target})
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(${target})
        endif()
        if(COMMAND umicom_target_qualification_inputs)
            umicom_target_qualification_inputs(${target} FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/codeguard_main.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_test_state.h")
        endif()
    endfunction()
    umicom_studio_codeguard_cli_target(umicom-studio-codeguard-cli-test)
    foreach(_case IN ITEMS empty finding default-root unicode extra-arguments missing-root empty-root bad-count null-vector null-root stdout-empty stdout-finding)
        add_test(NAME "studio.codeguard_cli.${_case}"
            COMMAND umicom-studio-codeguard-cli-test "${_case}")
        set_tests_properties("studio.codeguard_cli.${_case}" PROPERTIES
            TIMEOUT 60 LABELS "studio;codeguard;cli;output")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners("studio.codeguard_cli.${_case}"
                TARGETS umicom-studio-codeguard-cli-test)
        endif()
    endforeach()
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT APPLE AND NOT MSVC)
        umicom_studio_codeguard_cli_target(umicom-studio-codeguard-cli-fault-test)
        target_compile_definitions(umicom-studio-codeguard-cli-fault-test PRIVATE
            UMICOM_STUDIO_CODEGUARD_CLI_FAULTS=1)
        target_link_options(umicom-studio-codeguard-cli-fault-test PRIVATE
            -Wl,--wrap=umi_studio_codeguard_create -Wl,--wrap=umi_studio_codeguard_scan
            -Wl,--wrap=umi_studio_codeguard_destroy -Wl,--wrap=umi_codeguard_report_write
            -Wl,--wrap=UmiNativeArgumentsDispatch)
        foreach(_case IN ITEMS create scan report report-finding arguments)
            add_test(NAME "studio.codeguard_cli_fault.${_case}"
                COMMAND umicom-studio-codeguard-cli-fault-test "${_case}")
            set_tests_properties("studio.codeguard_cli_fault.${_case}" PROPERTIES
                TIMEOUT 60 LABELS "studio;codeguard;cli;output;fault")
            if(COMMAND umicom_test_impact_owners)
                umicom_test_impact_owners("studio.codeguard_cli_fault.${_case}"
                    TARGETS umicom-studio-codeguard-cli-fault-test)
            endif()
        endforeach()
    endif()
    # Invoke the actual process so Windows captures the Unicode command line,
    # rather than treating a synthetic callback vector as capture evidence.
    foreach(_case IN ITEMS empty unicode)
        if(_case STREQUAL "unicode")
            set(_leaf "codeguard-cli-é-日本")
        else()
            set(_leaf "codeguard-cli-empty")
        endif()
        set(_root "${CMAKE_CURRENT_BINARY_DIR}/test-fixtures/${_leaf}")
        file(MAKE_DIRECTORY "${_root}")
        add_test(NAME "studio.codeguard_cli_process.${_case}"
            COMMAND umicom-studio-codeguard "${_root}")
        set_tests_properties("studio.codeguard_cli_process.${_case}" PROPERTIES
            TIMEOUT 60 LABELS "studio;codeguard;cli;native-arguments")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners("studio.codeguard_cli_process.${_case}"
                TARGETS umicom-studio-codeguard)
        endif()
    endforeach()
endif()
