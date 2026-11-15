# Umicom Studio IDE
# File: cmake/UmicomStudioAiRecovery.cmake
#
# PURPOSE:
# Register native AI admission and session recovery scenarios using StudioCore.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT

if(BUILD_TESTING AND TARGET Umicom::StudioCore)
    foreach(_kind IN ITEMS configuration-admission session-recovery)
        string(REPLACE "-" "_" _source "${_kind}")
        set(_target "umicom-studio-ai-${_kind}-test")
        add_executable("${_target}" "tests/test_ai_${_source}.c")
        target_link_libraries("${_target}" PRIVATE Umicom::StudioCore)
        set_target_properties("${_target}" PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
        umicom_apply_warnings("${_target}")
        umicom_apply_sanitizers("${_target}")
    endforeach()
    foreach(_case IN ITEMS defaults unterminated required-empty optional-empty last-terminators
            remote-provider remote-model remote-secret remote-scheme remote-disabled
            remote-unconfigured remote-complete flags fitness-nan fitness-pos-inf fitness-neg-inf
            fitness-low fitness-high fitness-zero fitness-one numeric-bounds null-config null-output)
        set(_name "studio.ai.configuration.${_case}")
        add_test(NAME "${_name}" COMMAND umicom-studio-ai-configuration-admission-test "${_case}")
        set_tests_properties("${_name}" PROPERTIES TIMEOUT 30 LABELS "studio;ai;configuration")
        umicom_test_impact_owners("${_name}" TARGETS umicom-studio-ai-configuration-admission-test)
    endforeach()
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests/ai-session-recovery")
    foreach(_case IN ITEMS insert replace conflict stale invalid null-platform null-record
            null-report file malformed file-limit relative zero-budget policy)
        set(_name "studio.ai.session-recovery.${_case}")
        add_test(NAME "${_name}" COMMAND umicom-studio-ai-session-recovery-test "${_case}"
            "${CMAKE_CURRENT_BINARY_DIR}/tests/ai-session-recovery/${_case}.session")
        set_tests_properties("${_name}" PROPERTIES TIMEOUT 45 LABELS "studio;ai;session-recovery")
        umicom_test_impact_owners("${_name}" TARGETS umicom-studio-ai-session-recovery-test)
    endforeach()
endif()
