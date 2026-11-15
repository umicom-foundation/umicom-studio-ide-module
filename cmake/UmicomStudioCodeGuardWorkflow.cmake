#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/UmicomStudioCodeGuardWorkflow.cmake
#
# PURPOSE:
# Register isolated quality workflow and service lifetime qualification.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------

if(BUILD_TESTING)
    add_executable(umicom-studio-codeguard-workflow-test tests/test_codeguard_workflow.c)
    target_link_libraries(umicom-studio-codeguard-workflow-test PRIVATE
        Umicom::StudioCore PkgConfig::GLIB)
    target_compile_features(umicom-studio-codeguard-workflow-test PRIVATE c_std_23)
    umicom_apply_warnings(umicom-studio-codeguard-workflow-test)
    umicom_apply_sanitizers(umicom-studio-codeguard-workflow-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-codeguard-workflow-test)
    endif()
    foreach(_case IN ITEMS arguments closed idle success untrusted empty budget
        cancel-terminal close-terminal reopen-terminal switch-terminal trust-terminal
        graph-terminal snapshot-stale retry queue-stopped profile-copy cwd wrong-thread
        wrong-first-thread idle-cancel shutdown shutdown-terminal worker-release busy queued-cancel)
        add_test(NAME "studio.codeguard_workflow.${_case}"
            COMMAND umicom-studio-codeguard-workflow-test "${_case}")
        set_tests_properties("studio.codeguard_workflow.${_case}" PROPERTIES
            TIMEOUT 60 LABELS "studio;codeguard;workspace;ownership")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners("studio.codeguard_workflow.${_case}"
                TARGETS umicom-studio-codeguard-workflow-test)
        endif()
    endforeach()
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT APPLE AND NOT MSVC)
        add_executable(umicom-studio-codeguard-workflow-fault-test tests/test_codeguard_workflow.c)
        target_link_libraries(umicom-studio-codeguard-workflow-fault-test PRIVATE
            Umicom::StudioCore PkgConfig::GLIB)
        target_compile_features(umicom-studio-codeguard-workflow-fault-test PRIVATE c_std_23)
        target_compile_definitions(umicom-studio-codeguard-workflow-fault-test PRIVATE
            UMICOM_STUDIO_CODEGUARD_FAULTS=1)
        target_link_options(umicom-studio-codeguard-workflow-fault-test PRIVATE
            -Wl,--wrap=UmiCodeGuardReviewCreate -Wl,--wrap=UmiCodeGuardReviewPoll
            -Wl,--wrap=UmiCodeGuardReviewCancel -Wl,--wrap=UmiCodeGuardReviewDestroy)
        umicom_apply_warnings(umicom-studio-codeguard-workflow-fault-test)
        umicom_apply_sanitizers(umicom-studio-codeguard-workflow-fault-test)
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(umicom-studio-codeguard-workflow-fault-test)
        endif()
        foreach(_case IN ITEMS create-failure poll-before-admission cancel-shutdown-failure release-failure)
            add_test(NAME "studio.codeguard_workflow_fault.${_case}"
                COMMAND umicom-studio-codeguard-workflow-fault-test "${_case}")
            set_tests_properties("studio.codeguard_workflow_fault.${_case}" PROPERTIES
                TIMEOUT 60 LABELS "studio;codeguard;workspace;ownership;fault")
            if(COMMAND umicom_test_impact_owners)
                umicom_test_impact_owners("studio.codeguard_workflow_fault.${_case}"
                    TARGETS umicom-studio-codeguard-workflow-fault-test)
            endif()
        endforeach()
    endif()
    if(COMMAND umicom_target_qualification_inputs)
        umicom_target_qualification_inputs(umicom-studio-codeguard-workflow-test FILES
            "${CMAKE_CURRENT_LIST_DIR}/../src/app/services_codeguard.inc"
            "${CMAKE_CURRENT_LIST_DIR}/../src/app/services_lifecycle.inc"
            "${CMAKE_CURRENT_LIST_DIR}/../tests/studio_test_state.h")
    endif()
endif()
