/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/registered_command.inc
 *
 * PURPOSE:
 *   Execute canonical Studio commands through the authoritative Framework
 *   UmiCommandRegistry. GTK never calls developer business logic directly when
 *   a registered command already exists.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/test_discovery.h"
#include "umicom/terminal_ui/execution.h"

#include "umicom/studio/test_execution.h"
static bool RuntimeProgramConsoleBusy(UmiStudioGtkRuntimeChrome *runtime);
static int RuntimeTestRunContext(UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioTestRunContext *context);

static UmiCommandRegistry *runtime_registered_commands(
    UmiStudioGtkRuntimeChrome *runtime)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL) {
        return NULL;
    }
    return umi_ui_workbench_commands(
        umi_studio_ui_workbench(runtime->owner->ui));
}

/*
 * Provide the runtime registered command enabled operation used by this module and its
 * client applications.
 */
static int runtime_registered_command_enabled(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument)
{
    UmiCommandRegistry *commands = runtime_registered_commands(runtime);

    return commands != NULL &&
        command_id != NULL &&
        umi_command_registry_contains(commands, command_id) &&
        umi_command_registry_is_enabled(commands, command_id, argument);
}

/*
 * Provide the runtime execute registered command capture operation used by this module and
 * its client applications.
 */
/* Studio now prevents competing build/debug commands while the Framework console owns the selected executable. The prior dispatcher is retained for review. The previous implementation is retained for engineering review. */
#if 0
static UmiStatus runtime_execute_registered_command_capture(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument,
    char *out_message,
    size_t message_capacity)
{
    UmiCommandRegistry *commands;
    char local_message[512];
    char *message = out_message;
    size_t capacity = message_capacity;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || command_id == NULL ||
        command_id[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Canonical Build commands also appear in Search Everywhere and context
     * menus. They must use the same background workflow as the main menu,
     * rather than running a blocking compiler through the raw registry. */
    if (strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_CONFIGURE) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_COMPILE) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_TEST) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_CLEAN) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RUN) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_INSTALL) == 0) {
        if (argument == NULL || strcmp(argument, "background") != 0) {
            if (runtime->owner == NULL || runtime->owner->ui == NULL)
                return UMI_STATUS_INVALID_STATE;
            status = UmiDocumentCoordinatorSaveAll(
                umi_studio_ui_documents(runtime->owner->ui), NULL);
            if (status != UMI_STATUS_OK) {
                runtime_show_notification(runtime,
                    "Save unnamed documents with Save As and resolve file conflicts before building.");
                return status;
            }
        }
        argument = "background";
    }

    if (strcmp(command_id, "navigate.problem.next") == 0 ||
        strcmp(command_id, "navigate.problem.previous") == 0) {
        if (runtime->owner == NULL || runtime->owner->ui == NULL) return UMI_STATUS_INVALID_STATE;
        status = UmiStudioUiNavigateProblem(runtime->owner->ui,
            strcmp(command_id, "navigate.problem.previous") == 0);
        if (message != NULL && capacity != 0U) snprintf(message, capacity, "%s", umi_status_text(status));
        if (status != UMI_STATUS_OK) runtime_show_notification(runtime,
            "No available source location was opened. Read Problems and Output for details.");
        (void)runtime_refresh_services(runtime);
        return status;
    }

    commands = runtime_registered_commands(runtime);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (commands == NULL ||
        !umi_command_registry_contains(commands, command_id)) {
        runtime_show_notification(runtime, "Command is not registered.");
        return UMI_STATUS_NOT_FOUND;
    }

    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (!umi_command_registry_is_enabled(commands, command_id, argument)) {
        runtime_show_notification(runtime, "Command is currently disabled.");
        return UMI_STATUS_INVALID_STATE;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (message == NULL || capacity == 0U) {
        message = local_message;
        capacity = sizeof(local_message);
    }
    (void)memset(message, 0, capacity);

    UmiStudioTestService *backgroundDiscovery = NULL;
    if (strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_DISCOVER) == 0) {
        UmiStudioServices *services = runtime->owner != NULL && runtime->owner->ui != NULL
            ? umi_studio_ui_services(runtime->owner->ui) : NULL;
        UmiStudioWorkspaceSnapshot workspace;
        const UmiBuildProfile *profile = services != NULL
            ? umi_studio_build_service_profile(umi_studio_services_build(services)) : NULL;
        if (services == NULL || profile == NULL ||
            umi_studio_workspace_snapshot(services, &workspace) != UMI_STATUS_OK ||
            !workspace.graph.open || !workspace.graph.trusted) {
            runtime_show_notification(runtime, "Open and trust the workspace before discovering executable CTest scripts.");
            return UMI_STATUS_INVALID_STATE;
        }
        /* A toolbar may still carry a previous catalogue's path. Use the active
         * profile for a GUI discovery request; the CLI retains explicit paths. */
        argument = profile->build_directory;
        backgroundDiscovery = umi_studio_services_tests(services);
/* The Discover command now captures the same tool choice used by build and test phases. The previous implementation is retained for engineering review. */
#if 0
        status = UmiStudioTestDiscoveryArm(backgroundDiscovery,
            umi_studio_services_task_queue(services), workspace.graph.revision);
#endif
        status = UmiStudioTestDiscoveryArmWithToolDirectory(backgroundDiscovery,
            umi_studio_services_task_queue(services), workspace.graph.revision,
            profile->tool_directory);
        if (status != UMI_STATUS_OK) {
            runtime_show_notification(runtime, "Discovery is already pending or its task queue is unavailable.");
            return status;
        }
    }

    UmiStudioTestService *backgroundRun = NULL;
    if (strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_RUN_ALL) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_RUN_SELECTED) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_RERUN_FAILED) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_REPEAT_SELECTED) == 0) {
        UmiStudioTestRunContext context;
        if (!RuntimeTestRunContext(runtime, &context)) {
            runtime_show_notification(runtime, "Open and trust the active workspace before running tests.");
            return UMI_STATUS_PERMISSION_DENIED;
        }
        UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
        backgroundRun = umi_studio_services_tests(services);
        status = UmiStudioTestRunArm(backgroundRun,
            umi_studio_services_task_queue(services), &context);
        if (status != UMI_STATUS_OK) {
            runtime_show_notification(runtime, "Another test operation is pending or its task queue is unavailable.");
            return status;
        }
    }

    /* Arming changes execution mode only. The same authoritative registry still
     * checks workspace trust and process permissions before calling the handler. */
    UmiTerminalController *backgroundTerminal = NULL;
    if (strcmp(command_id, UMI_STUDIO_COMMAND_TERMINAL_EXECUTE) == 0) {
        UmiStudioServices *services = runtime->owner != NULL && runtime->owner->ui != NULL
            ? umi_studio_ui_services(runtime->owner->ui) : NULL;
        backgroundTerminal = umi_studio_services_terminal_controller(services);
        status = UmiTerminalControllerArmBackground(backgroundTerminal);
        if (status != UMI_STATUS_OK) {
            runtime_show_notification(runtime, "A terminal command is already pending. Stop it or wait.");
            return status;
        }
    }
    runtime->interaction_busy = 1;
    status = umi_command_registry_execute(
        commands,
        command_id,
        argument,
        message,
        capacity);
    runtime->interaction_busy = 0;
    if (backgroundTerminal != NULL)
        UmiTerminalControllerDisarmBackground(backgroundTerminal);
    if (backgroundDiscovery != NULL) {
        UmiStudioTestDiscoveryDisarm(backgroundDiscovery);
        if (status == UMI_STATUS_OK && UmiStudioTestDiscoveryPending(backgroundDiscovery))
            (void)snprintf(message, capacity, "Test discovery started in the background. Stop retains the previous catalogue.");
        else if (status != UMI_STATUS_OK)
            (void)snprintf(message, capacity, "Test discovery was not started: %s", umi_status_text(status));
    }

    if (backgroundRun != NULL) {
        UmiStudioTestRunDisarm(backgroundRun);
        if (status == UMI_STATUS_OK && UmiStudioTestRunPending(backgroundRun))
            (void)snprintf(message, capacity,
                "Test run started. Stop cancels the active CTest request and prevents later attempts.");
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (message[0] != '\0') {
        runtime_show_notification(runtime, message);
    } else /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }

    (void)runtime_refresh_services(runtime);
    return status;
}
#endif
/* Save and command observers can release the window during dispatch. Retain this route for review; the replacement copies arguments and reacquires its weak host before completion work. */
#if 0
static UmiStatus runtime_execute_registered_command_capture(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument,
    char *out_message,
    size_t message_capacity)
{
    UmiCommandRegistry *commands;
    char local_message[512];
    char *message = out_message;
    size_t capacity = message_capacity;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || command_id == NULL ||
        command_id[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Rebuilding or starting a debugger against a running console program can
     * replace its selected artifact. Require its native cleanup to finish first,
     * including after its separate window has already closed. */
    if (RuntimeProgramConsoleBusy(runtime) &&
        (strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_CONFIGURE) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_COMPILE) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_TEST) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_CLEAN) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RUN) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RUN_CURRENT) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_INSTALL) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_PACKAGE) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_REBUILD) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_DEPLOY) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RETRY) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RUN_NEXT) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RUN_ALL) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_DEBUG_INITIALIZE) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_DEBUG_START) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_DEBUG_RESTART) == 0)) {
        runtime_show_notification(runtime, "Stop the program console before building or debugging.");
        return UMI_STATUS_BUSY;
    }

    /* Canonical Build commands also appear in Search Everywhere and context
     * menus. They must use the same background workflow as the main menu,
     * rather than running a blocking compiler through the raw registry. */
    if (strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_CONFIGURE) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_COMPILE) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_TEST) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_CLEAN) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RUN) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_INSTALL) == 0) {
        if (argument == NULL || strcmp(argument, "background") != 0) {
            if (runtime->owner == NULL || runtime->owner->ui == NULL)
                return UMI_STATUS_INVALID_STATE;
            status = UmiDocumentCoordinatorSaveAll(
                umi_studio_ui_documents(runtime->owner->ui), NULL);
            if (status != UMI_STATUS_OK) {
                runtime_show_notification(runtime,
                    "Save unnamed documents with Save As and resolve file conflicts before building.");
                return status;
            }
        }
        argument = "background";
    }

    if (strcmp(command_id, "navigate.problem.next") == 0 ||
        strcmp(command_id, "navigate.problem.previous") == 0) {
        if (runtime->owner == NULL || runtime->owner->ui == NULL) return UMI_STATUS_INVALID_STATE;
        status = UmiStudioUiNavigateProblem(runtime->owner->ui,
            strcmp(command_id, "navigate.problem.previous") == 0);
        if (message != NULL && capacity != 0U) snprintf(message, capacity, "%s", umi_status_text(status));
        if (status != UMI_STATUS_OK) runtime_show_notification(runtime,
            "No available source location was opened. Read Problems and Output for details.");
        (void)runtime_refresh_services(runtime);
        return status;
    }

    commands = runtime_registered_commands(runtime);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (commands == NULL ||
        !umi_command_registry_contains(commands, command_id)) {
        runtime_show_notification(runtime, "Command is not registered.");
        return UMI_STATUS_NOT_FOUND;
    }

    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (!umi_command_registry_is_enabled(commands, command_id, argument)) {
        runtime_show_notification(runtime, "Command is currently disabled.");
        return UMI_STATUS_INVALID_STATE;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (message == NULL || capacity == 0U) {
        message = local_message;
        capacity = sizeof(local_message);
    }
    (void)memset(message, 0, capacity);

    UmiStudioTestService *backgroundDiscovery = NULL;
    if (strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_DISCOVER) == 0) {
        UmiStudioServices *services = runtime->owner != NULL && runtime->owner->ui != NULL
            ? umi_studio_ui_services(runtime->owner->ui) : NULL;
        UmiStudioWorkspaceSnapshot workspace;
        const UmiBuildProfile *profile = services != NULL
            ? umi_studio_build_service_profile(umi_studio_services_build(services)) : NULL;
        if (services == NULL || profile == NULL ||
            umi_studio_workspace_snapshot(services, &workspace) != UMI_STATUS_OK ||
            !workspace.graph.open || !workspace.graph.trusted) {
            runtime_show_notification(runtime, "Open and trust the workspace before discovering executable CTest scripts.");
            return UMI_STATUS_INVALID_STATE;
        }
        /* A toolbar may still carry a previous catalogue's path. Use the active
         * profile for a GUI discovery request; the CLI retains explicit paths. */
        argument = profile->build_directory;
        backgroundDiscovery = umi_studio_services_tests(services);
/* The Discover command now captures the same tool choice used by build and test phases. The previous implementation is retained for engineering review. */
#if 0
        status = UmiStudioTestDiscoveryArm(backgroundDiscovery,
            umi_studio_services_task_queue(services), workspace.graph.revision);
#endif
        status = UmiStudioTestDiscoveryArmWithToolDirectory(backgroundDiscovery,
            umi_studio_services_task_queue(services), workspace.graph.revision,
            profile->tool_directory);
        if (status != UMI_STATUS_OK) {
            runtime_show_notification(runtime, "Discovery is already pending or its task queue is unavailable.");
            return status;
        }
    }

    UmiStudioTestService *backgroundRun = NULL;
    if (strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_RUN_ALL) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_RUN_SELECTED) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_RERUN_FAILED) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_REPEAT_SELECTED) == 0) {
        UmiStudioTestRunContext context;
        if (!RuntimeTestRunContext(runtime, &context)) {
            runtime_show_notification(runtime, "Open and trust the active workspace before running tests.");
            return UMI_STATUS_PERMISSION_DENIED;
        }
        UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
        backgroundRun = umi_studio_services_tests(services);
        status = UmiStudioTestRunArm(backgroundRun,
            umi_studio_services_task_queue(services), &context);
        if (status != UMI_STATUS_OK) {
            runtime_show_notification(runtime, "Another test operation is pending or its task queue is unavailable.");
            return status;
        }
    }

    /* Arming changes execution mode only. The same authoritative registry still
     * checks workspace trust and process permissions before calling the handler. */
    /* Enforcement clarification: raw registry dispatch retains descriptor
     * metadata but does not enforce trust flags. terminal_execute_handler
     * checks the shared workspace snapshot before either execution mode can
     * create a process, and disarms a rejected background request. */
    UmiTerminalController *backgroundTerminal = NULL;
    if (strcmp(command_id, UMI_STUDIO_COMMAND_TERMINAL_EXECUTE) == 0) {
        UmiStudioServices *services = runtime->owner != NULL && runtime->owner->ui != NULL
            ? umi_studio_ui_services(runtime->owner->ui) : NULL;
        backgroundTerminal = umi_studio_services_terminal_controller(services);
        status = UmiTerminalControllerArmBackground(backgroundTerminal);
        if (status != UMI_STATUS_OK) {
            runtime_show_notification(runtime, "A terminal command is already pending. Stop it or wait.");
            return status;
        }
    }
    runtime->interaction_busy = 1;
    status = umi_command_registry_execute(
        commands,
        command_id,
        argument,
        message,
        capacity);
    runtime->interaction_busy = 0;
    if (backgroundTerminal != NULL)
        UmiTerminalControllerDisarmBackground(backgroundTerminal);
    if (backgroundDiscovery != NULL) {
        UmiStudioTestDiscoveryDisarm(backgroundDiscovery);
        if (status == UMI_STATUS_OK && UmiStudioTestDiscoveryPending(backgroundDiscovery))
            (void)snprintf(message, capacity, "Test discovery started in the background. Stop retains the previous catalogue.");
        else if (status != UMI_STATUS_OK)
            (void)snprintf(message, capacity, "Test discovery was not started: %s", umi_status_text(status));
    }

    if (backgroundRun != NULL) {
        UmiStudioTestRunDisarm(backgroundRun);
        if (status == UMI_STATUS_OK && UmiStudioTestRunPending(backgroundRun))
            (void)snprintf(message, capacity,
                "Test run started. Stop cancels the active CTest request and prevents later attempts.");
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (message[0] != '\0') {
        runtime_show_notification(runtime, message);
    } else /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }

    (void)runtime_refresh_services(runtime);
    return status;
}

#endif
/* Saving may invoke a document provider. Capture project and profile identity,
 * then recheck both before the caller dispatches its tool. Framework continues
 * to own draft saving and profile equality; Studio supplies workspace authority. */
static UmiStatus RuntimeSaveProjectSources(UmiStudioGtkRuntimeChrome *runtime,
    char *message, size_t capacity)
{
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStatus status = UmiStudioBuildCheckAdmission(services, message, capacity);
    if (status != UMI_STATUS_OK) return status;
    if (UmiGtk4AdapterDocumentSaveAllBusy(runtime->owner->adapter) ||
        UmiGtk4AdapterCloseDocumentsBusy(runtime->owner->adapter)) {
        (void)snprintf(message, capacity, "Finish or cancel the active document save before starting tools.");
        return UMI_STATUS_BUSY;
    }
    UmiStudioWorkspaceSnapshot workspace;
    status = umi_studio_workspace_snapshot(services, &workspace);
    if (status != UMI_STATUS_OK) return status;
    UmiBuildProfile profile = *umi_studio_build_service_profile(umi_studio_services_build(services));
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = UmiDocumentCoordinatorSaveAll(umi_studio_ui_documents(runtime->owner->ui), NULL);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) status = UMI_STATUS_CANCELLED;
    else if (status != UMI_STATUS_OK) {
        (void)snprintf(message, capacity,
            "Tools did not start. Save unnamed documents with Save As and resolve file conflicts first.");
    } else {
        services = umi_studio_ui_services(runtime->owner->ui);
        UmiStudioWorkspaceSnapshot current;
        status = umi_studio_workspace_snapshot(services, &current);
        if (status == UMI_STATUS_OK && (!current.graph.open || !current.graph.trusted ||
            current.graph.revision != workspace.graph.revision ||
            !umi_path_equal(current.graph.root, workspace.graph.root) ||
            !umi_build_profile_equal(&profile,
                umi_studio_build_service_profile(umi_studio_services_build(services))))) {
            status = UMI_STATUS_INVALID_STATE;
            (void)snprintf(message, capacity,
                "The project or its settings changed while saving. Review the active project before starting tools again.");
        }
        if (status == UMI_STATUS_OK)
            status = UmiStudioBuildCheckAdmission(services, message, capacity);
        if (status == UMI_STATUS_OK && RuntimeProgramConsoleBusy(runtime)) {
            status = UMI_STATUS_BUSY;
            (void)snprintf(message, capacity, "Stop the program console before building or debugging.");
        }
    }
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}

static UmiStatus runtime_execute_registered_command_capture(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument,
    char *out_message,
    size_t message_capacity)
{
    UmiCommandRegistry *commands;
    char local_message[512];
    char *message = out_message;
    size_t capacity = message_capacity;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || command_id == NULL ||
        command_id[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    UmiStudioFileDialogRequest *ownerRequest = runtime_file_dialog_request_new(runtime, 0U);
    if (ownerRequest == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GObject *ownerWindow = NULL;
    char *ownedCommand = g_strdup(command_id);
    char *ownedArgument = g_strdup(argument);
    command_id = ownedCommand;
    argument = ownedArgument;

    /* Rebuilding or starting a debugger against a running console program can
     * replace its selected artifact. Require its native cleanup to finish first,
     * including after its separate window has already closed. */
    if (RuntimeProgramConsoleBusy(runtime) &&
        (strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_CONFIGURE) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_COMPILE) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_TEST) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_CLEAN) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RUN) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RUN_CURRENT) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_INSTALL) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_PACKAGE) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_REBUILD) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_DEPLOY) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RETRY) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RUN_NEXT) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RUN_ALL) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_DEBUG_INITIALIZE) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_DEBUG_START) == 0 ||
         strcmp(command_id, UMI_STUDIO_COMMAND_DEBUG_RESTART) == 0)) {
        runtime_show_notification(runtime, "Stop the program console before building or debugging.");
        { status = UMI_STATUS_BUSY; goto command_finished; }
    }

/* A single checked save boundary now covers menu, toolbar and canonical command execution. Preserve the earlier branches for review; the background argument previously skipped saving and admission followed disk writes. */
#if 0
    /* Debug launches can originate in command search as well as the main menu.
     * Save through Framework's coordinator once, then resolve the weak host again:
     * a saved-document observer may have closed the window or changed projects. */
    if (strcmp(command_id, UMI_STUDIO_COMMAND_DEBUG_START) == 0) {
        UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
        UmiStudioWorkspaceSnapshot workspace;
        status = umi_studio_workspace_snapshot(services, &workspace);
        if (status != UMI_STATUS_OK) goto command_finished;
        if (!workspace.graph.open || !workspace.graph.trusted) {
            status = UMI_STATUS_PERMISSION_DENIED;
            runtime_show_notification(runtime, "Open a project and enable its build tools in Project Settings before debugging.");
            goto command_finished;
        }
        if (UmiStudioBuildBusy(umi_studio_services_build(services)) ||
            UmiStudioDebuggerNativeBusy(umi_studio_services_debugger(services)) ||
            UmiGtk4AdapterDocumentSaveAllBusy(runtime->owner->adapter) ||
            UmiGtk4AdapterCloseDocumentsBusy(runtime->owner->adapter)) {
            status = UMI_STATUS_BUSY;
            runtime_show_notification(runtime, "Finish the active build, debugger or document save before starting Debug.");
            goto command_finished;
        }
        status = UmiDocumentCoordinatorSaveAll(umi_studio_ui_documents(runtime->owner->ui), NULL);
        runtime = runtime_file_dialog_owner(ownerRequest, &ownerWindow);
        if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto command_finished; }
        if (status != UMI_STATUS_OK) {
            runtime_show_notification(runtime, "Debug did not start. Save unnamed documents with Save As and resolve file conflicts first.");
            goto command_finished;
        }
        UmiStudioWorkspaceSnapshot current;
        status = umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui), &current);
        if (status == UMI_STATUS_OK && (!current.graph.open || !current.graph.trusted ||
            current.graph.revision != workspace.graph.revision ||
            !umi_path_equal(current.graph.root, workspace.graph.root))) status = UMI_STATUS_INVALID_STATE;
        if (status != UMI_STATUS_OK) {
            runtime_show_notification(runtime, "The project changed while saving. Review the active project before starting Debug again.");
            goto command_finished;
        }
    }

    /* Canonical Build commands also appear in Search Everywhere and context
     * menus. They must use the same background workflow as the main menu,
     * rather than running a blocking compiler through the raw registry. */
    if (strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_CONFIGURE) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_COMPILE) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_TEST) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_CLEAN) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RUN) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_INSTALL) == 0) {
        if (argument == NULL || strcmp(argument, "background") != 0) {
            if (runtime->owner == NULL || runtime->owner->ui == NULL)
                { status = UMI_STATUS_INVALID_STATE; goto command_finished; }
            status = UmiDocumentCoordinatorSaveAll(
                umi_studio_ui_documents(runtime->owner->ui), NULL);
            runtime = runtime_file_dialog_owner(ownerRequest, &ownerWindow);
            if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto command_finished; }
            if (status != UMI_STATUS_OK) {
                runtime_show_notification(runtime,
                    "Save unnamed documents with Save As and resolve file conflicts before building.");
                goto command_finished;
            }
        }
        argument = "background";
    }


#endif
    /* Admission must precede source writes for every graphical entry route.
     * The background argument chooses scheduling only; it cannot bypass saving
     * or carry authority from a previous project. Run Current intentionally
     * launches existing output without saving or building. */
    int buildSources = strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_CONFIGURE) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_COMPILE) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_TEST) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_CLEAN) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_RUN) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_INSTALL) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_PACKAGE) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_REBUILD) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_BUILD_DEPLOY) == 0;
    if (buildSources || strcmp(command_id, UMI_STUDIO_COMMAND_DEBUG_START) == 0) {
        char saveMessage[512] = {0};
        status = RuntimeSaveProjectSources(runtime, saveMessage, sizeof saveMessage);
        runtime = runtime_file_dialog_owner(ownerRequest, &ownerWindow);
        if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto command_finished; }
        if (status != UMI_STATUS_OK) {
            if (message != NULL && capacity > 0U)
                (void)snprintf(message, capacity, "%s", saveMessage);
            runtime_show_notification(runtime, saveMessage[0] != '\0'
                ? saveMessage : umi_status_text(status));
            goto command_finished;
        }
        if (buildSources) argument = "background";
    }

    if (strcmp(command_id, "navigate.problem.next") == 0 ||
        strcmp(command_id, "navigate.problem.previous") == 0) {
        status = RuntimeNavigateProblem(runtime, strcmp(command_id, "navigate.problem.previous") == 0);
        if (message != NULL && capacity != 0U) snprintf(message, capacity, "%s", umi_status_text(status));
        goto command_finished;
    }

    /* Every graphical route to Refresh keeps recursive enumeration off GTK.
     * The registry retains its synchronous form for existing non-GUI clients. */
    if (strcmp(command_id, UMI_STUDIO_COMMAND_WORKSPACE_REFRESH) == 0)
        argument = "background";

    commands = runtime_registered_commands(runtime);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (commands == NULL ||
        !umi_command_registry_contains(commands, command_id)) {
        runtime_show_notification(runtime, "Command is not registered.");
        { status = UMI_STATUS_NOT_FOUND; goto command_finished; }
    }

    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (!umi_command_registry_is_enabled(commands, command_id, argument)) {
        runtime_show_notification(runtime, "Command is currently disabled.");
        { status = UMI_STATUS_INVALID_STATE; goto command_finished; }
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (message == NULL || capacity == 0U) {
        message = local_message;
        capacity = sizeof(local_message);
    }
    (void)memset(message, 0, capacity);

    UmiStudioTestService *backgroundDiscovery = NULL;
    if (strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_DISCOVER) == 0) {
        UmiStudioServices *services = runtime->owner != NULL && runtime->owner->ui != NULL
            ? umi_studio_ui_services(runtime->owner->ui) : NULL;
        UmiStudioWorkspaceSnapshot workspace;
        const UmiBuildProfile *profile = services != NULL
            ? umi_studio_build_service_profile(umi_studio_services_build(services)) : NULL;
        if (services == NULL || profile == NULL ||
            umi_studio_workspace_snapshot(services, &workspace) != UMI_STATUS_OK ||
            !workspace.graph.open || !workspace.graph.trusted) {
            runtime_show_notification(runtime, "Open and trust the workspace before discovering executable CTest scripts.");
            { status = UMI_STATUS_INVALID_STATE; goto command_finished; }
        }
        /* A toolbar may still carry a previous catalogue's path. Use the active
         * profile for a GUI discovery request; the CLI retains explicit paths. */
        argument = profile->build_directory;
        backgroundDiscovery = umi_studio_services_tests(services);
/* The Discover command now captures the same tool choice used by build and test phases. The previous implementation is retained for engineering review. */
#if 0
        status = UmiStudioTestDiscoveryArm(backgroundDiscovery,
            umi_studio_services_task_queue(services), workspace.graph.revision);
#endif
        status = UmiStudioTestDiscoveryArmWithToolDirectory(backgroundDiscovery,
            umi_studio_services_task_queue(services), workspace.graph.revision,
            profile->tool_directory);
        if (status != UMI_STATUS_OK) {
            runtime_show_notification(runtime, "Discovery is already pending or its task queue is unavailable.");
            goto command_finished;
        }
    }

    UmiStudioTestService *backgroundRun = NULL;
    if (strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_RUN_ALL) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_RUN_SELECTED) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_RERUN_FAILED) == 0 ||
        strcmp(command_id, UMI_STUDIO_COMMAND_TESTS_REPEAT_SELECTED) == 0) {
        UmiStudioTestRunContext context;
        if (!RuntimeTestRunContext(runtime, &context)) {
            runtime_show_notification(runtime, "Open and trust the active workspace before running tests.");
            { status = UMI_STATUS_PERMISSION_DENIED; goto command_finished; }
        }
        UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
        backgroundRun = umi_studio_services_tests(services);
        status = UmiStudioTestRunArm(backgroundRun,
            umi_studio_services_task_queue(services), &context);
        if (status != UMI_STATUS_OK) {
            runtime_show_notification(runtime, "Another test operation is pending or its task queue is unavailable.");
            goto command_finished;
        }
    }

    /* Arming changes execution mode only. The same authoritative registry still
     * checks workspace trust and process permissions before calling the handler. */
    /* Enforcement clarification: raw registry dispatch retains descriptor
     * metadata but does not enforce trust flags. terminal_execute_handler
     * checks the shared workspace snapshot before either execution mode can
     * create a process, and disarms a rejected background request. */
    UmiTerminalController *backgroundTerminal = NULL;
    if (strcmp(command_id, UMI_STUDIO_COMMAND_TERMINAL_EXECUTE) == 0) {
        UmiStudioServices *services = runtime->owner != NULL && runtime->owner->ui != NULL
            ? umi_studio_ui_services(runtime->owner->ui) : NULL;
        backgroundTerminal = umi_studio_services_terminal_controller(services);
        status = UmiTerminalControllerArmBackground(backgroundTerminal);
        if (status != UMI_STATUS_OK) {
            runtime_show_notification(runtime, "A terminal command is already pending. Stop it or wait.");
            goto command_finished;
        }
    }
    runtime->interaction_busy = 1;
    status = umi_command_registry_execute(
        commands,
        command_id,
        argument,
        message,
        capacity);
    g_clear_object(&ownerWindow);
    runtime = runtime_file_dialog_owner(ownerRequest, &ownerWindow);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto command_finished; }
    runtime->interaction_busy = 0;
    if (backgroundTerminal != NULL)
        UmiTerminalControllerDisarmBackground(backgroundTerminal);
/* Keep the canonical discovery handler's actionable explanation. The previous generic replacement is retained for review. */
#if 0
    if (backgroundDiscovery != NULL) {
        UmiStudioTestDiscoveryDisarm(backgroundDiscovery);
        if (status == UMI_STATUS_OK && UmiStudioTestDiscoveryPending(backgroundDiscovery))
            (void)snprintf(message, capacity, "Test discovery started in the background. Stop retains the previous catalogue.");
        else if (status != UMI_STATUS_OK)
            (void)snprintf(message, capacity, "Test discovery was not started: %s", umi_status_text(status));
    }
#endif
    if (backgroundDiscovery != NULL) {
        UmiStudioTestDiscoveryDisarm(backgroundDiscovery);
        if (status == UMI_STATUS_OK && UmiStudioTestDiscoveryPending(backgroundDiscovery))
            (void)snprintf(message, capacity, "Test discovery started in the background. Stop retains the previous catalogue.");
        else if (status != UMI_STATUS_OK && message[0] == '\0')
            (void)snprintf(message, capacity, "Test discovery was not started: %s", umi_status_text(status));
    }

    if (backgroundRun != NULL) {
        UmiStudioTestRunDisarm(backgroundRun);
        if (status == UMI_STATUS_OK && UmiStudioTestRunPending(backgroundRun))
            (void)snprintf(message, capacity,
                "Test run started. Stop cancels the active CTest request and prevents later attempts.");
    }

    /* Refresh before feedback, then resolve the host again. Notifications may
     * synchronously close the workbench; no runtime read may follow them. */
    (void)runtime_refresh_services(runtime);
    g_clear_object(&ownerWindow);
    runtime = runtime_file_dialog_owner(ownerRequest, &ownerWindow);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto command_finished; }
    /* Apply this branch only when its contract condition is satisfied. */
    if (message[0] != '\0') {
        runtime_show_notification(runtime, message);
    } else /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }

command_finished:
    g_clear_object(&ownerWindow);
    runtime_file_dialog_request_free(ownerRequest);
    g_free(ownedCommand);
    g_free(ownedArgument);
    return status;
}

/*
 * Provide the runtime execute registered command operation used by this module and its
 * client applications.
 */
static UmiStatus runtime_execute_registered_command(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument)
{
    return runtime_execute_registered_command_capture(
        runtime,
        command_id,
        argument,
        NULL,
        0U);
}
