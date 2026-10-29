#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/UmicomStudioTestIsolation.cmake
#
# PURPOSE:
#   Declare dependencies and coverage for tests with privately owned local state.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------
if(BUILD_TESTING)
    # Keep the explicit allowlist aligned with sources using studio_test_state.h.
    # Existing CTest resource locks remain until broader parallel qualification.
    # The original list used a hyphen for extension_centre, but that target
    # retains underscores. Preserve it for review; checked target names below
    # provide GLib includes and qualification dependencies to every migrated test.
    if(FALSE)
    foreach(_target IN ITEMS
            umicom-studio-ai-workspace-test
            umicom-studio-automation-audit-test
            umicom-studio-bootstrap-inventory-test
            umicom-studio-commands-test
            umicom-studio-command-inventory-test
            umicom-studio-command-inventory-partition-test
            umicom-studio-composition-lifecycle-test
            umicom-studio-designer-command-contribution-test
            umicom-studio-diagnostics-pipeline-test
            umicom-studio-documents-test
            umicom-studio-document-workbench-test
            umicom-studio-extension-centre-test
            umicom-studio-platform-contract-test
            umicom-studio-registry-extension-compatibility-test
            umicom-studio-service-inventory-test
            umicom-studio-service-namespace-inventory-test
            umicom-studio-state-test
            umicom-studio-state-inventory-consistency-test
            umicom-studio-tasks-test
            umicom-studio-terminal-workbench-test
            umicom-studio-services-test)
    endforeach()
    endif()
    foreach(_target IN ITEMS
            umicom-studio-ai-workspace-test
            umicom-studio-application_authority-test
            umicom-studio-automation-audit-test
            umicom-studio-bootstrap-inventory-test
            umicom-studio-build-workspace-views-test
            umicom-studio-command-inventory-partition-test
            umicom-studio-command-inventory-test
            umicom-studio-commands-test
            umicom-studio-composition-lifecycle-test
            umicom-studio-contributions-test
            umicom-studio-data-test
            umicom-studio-dead-letters-test
            umicom-studio-delivery-services-test
            umicom-studio-designer-command-contribution-test
            umicom-studio-desktop_shell-test
            umicom-studio-diagnostics-pipeline-test
            umicom-studio-diagnostics-test
            umicom-studio-document-workbench-test
            umicom-studio-documents-test
            umicom-studio-editor_workbench_commands-test
            umicom-studio-extension_centre-test
            umicom-studio-fabric-test
            umicom-studio-federated_content-test
            umicom-studio-federated_interactions-test
            umicom-studio-federated_workspace_session-test
            umicom-studio-files-test
            umicom-studio-framework-bootstrap-test
            umicom-studio-history-test
            umicom-studio-messages-test
            umicom-studio-observability-test
            umicom-studio-operations-test
            umicom-studio-outbox-test
            umicom-studio-persistence-test
            umicom-studio-perspectives-test
            umicom-studio-platform-contract-test
            umicom-studio-platform-test
            umicom-studio-plugins-test
            umicom-studio-processes-test
            umicom-studio-registry-extension-compatibility-test
            umicom-studio-replay-test
            umicom-studio-resilience-test
            umicom-studio-search-test
            umicom-studio-security-test
            umicom-studio-service-inventory-test
            umicom-studio-service-namespace-inventory-test
            umicom-studio-services-test
            umicom-studio-source_control_workbench-test
            umicom-studio-state-inventory-consistency-test
            umicom-studio-state-test
            umicom-studio-tasks-test
            umicom-studio-terminal-workbench-test
            umicom-studio-test-workspace-views-test
            umicom-studio-trading-workspace-views-test
            umicom-studio-ui-test
            umicom-studio-ui_actions-test
            umicom-studio-ui_catalogue-test
            umicom-studio-ui_headless-test
            umicom-studio-ui_services-test
            umicom-studio-ui_session-test
            umicom-studio-view_models-test
            umicom-studio-watcher-test
            umicom-studio-workbench-test
            umicom-studio-workbench_activity-test
            umicom-studio-workbench_keybinding-test
            umicom-studio-workbench_shell_catalogue-test
            umicom-studio-workbench_views-test
            umicom-studio-workspace-test)
        # These targets are all declared above under BUILD_TESTING. A renamed
        # target must not silently lose its utility includes or source ownership.
        if(NOT TARGET "${_target}")
            message(FATAL_ERROR "Studio test isolation target is missing: ${_target}")
        endif()
        if(TARGET "${_target}")
            target_link_libraries("${_target}" PRIVATE PkgConfig::GLIB)
            if(COMMAND umicom_target_qualification_inputs)
                umicom_target_qualification_inputs("${_target}" FILES
                    "${CMAKE_CURRENT_LIST_DIR}/../tests/studio_test_state.h")
            endif()
        endif()
    endforeach()
    add_executable(umicom-studio-state-isolation-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_state_isolation.c")
    target_link_libraries(umicom-studio-state-isolation-test PRIVATE PkgConfig::GLIB)
    umicom_apply_warnings(umicom-studio-state-isolation-test)
    umicom_apply_sanitizers(umicom-studio-state-isolation-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-state-isolation-test)
    endif()
    foreach(_case IN ITEMS unset existing empty nested repeat)
        add_test(NAME studio.state_isolation.${_case}
            COMMAND umicom-studio-state-isolation-test "${_case}")
        set_tests_properties(studio.state_isolation.${_case} PROPERTIES
            TIMEOUT 30 LABELS "studio;test-state;ownership;regression")
    endforeach()
    if(COMMAND umicom_target_qualification_inputs)
        umicom_target_qualification_inputs(umicom-studio-state-isolation-test FILES
            "${CMAKE_CURRENT_LIST_DIR}/../tests/studio_test_state.h")
    endif()
endif()
