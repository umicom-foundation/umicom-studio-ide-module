/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/registered_command.inc
 *
 * PURPOSE:
 *   Execute canonical Studio commands through the authoritative Framework
 *   UmiCommandRegistry. GTK never calls developer business logic directly when
 *   a registered command already exists.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static UmiCommandRegistry *runtime_registered_commands(
    UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL) {
        return NULL;
    }
    return umi_ui_workbench_commands(
        umi_studio_ui_workbench(runtime->owner->ui));
}

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

    if (runtime == NULL || command_id == NULL ||
        command_id[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    commands = runtime_registered_commands(runtime);
    if (commands == NULL ||
        !umi_command_registry_contains(commands, command_id)) {
        runtime_show_notification(runtime, "Command is not registered.");
        return UMI_STATUS_NOT_FOUND;
    }

    if (!umi_command_registry_is_enabled(commands, command_id, argument)) {
        runtime_show_notification(runtime, "Command is currently disabled.");
        return UMI_STATUS_INVALID_STATE;
    }

    if (message == NULL || capacity == 0U) {
        message = local_message;
        capacity = sizeof(local_message);
    }
    (void)memset(message, 0, capacity);

    runtime->interaction_busy = 1;
    status = umi_command_registry_execute(
        commands,
        command_id,
        argument,
        message,
        capacity);
    runtime->interaction_busy = 0;

    if (message[0] != '\0') {
        runtime_show_notification(runtime, message);
    } else if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }

    (void)runtime_refresh_services(runtime);
    return status;
}

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
