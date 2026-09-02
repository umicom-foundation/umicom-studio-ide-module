/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/operation_catalogue.inc
 *
 * PURPOSE:
 *   Make the canonical Studio command catalogue operational in Search
 *   Everywhere. Entries are displayed only when the real command exists in
 *   UmiCommandRegistry. Required arguments are collected by GTK and handed
 *   directly to that registered command.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static const char *runtime_operation_argument_message(
    UmiStudioGtkOperationArgumentKind kind)
{
    /* Select the behaviour associated with the requested command or state value. */
    switch (kind) {
        case UMI_STUDIO_GTK_OPERATION_ARGUMENT_PATH:
            return "Enter the repository/workspace path required by this command.";
        case UMI_STUDIO_GTK_OPERATION_ARGUMENT_THREAD:
            return "Enter the debugger thread id.";
        case UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEST:
            return "Enter the stable Test Explorer item id.";
        case UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT:
            return "Enter the command argument.";
        case UMI_STUDIO_GTK_OPERATION_ARGUMENT_OPTIONAL:
            return "Enter an optional argument, or cancel to run without one.";
        case UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE:
        default:
            return "";
    }
}

/*
 * Provide the runtime pending operation accepted operation used by this module and its
 * client applications.
 */
static UmiStatus runtime_pending_operation_accepted(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *value)
{
    char command_id[UMI_COMMAND_ID_CAPACITY];

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL ||
        runtime->pending_operation_command[0] == '\0') {
        return UMI_STATUS_INVALID_STATE;
    }

    (void)snprintf(
        command_id,
        sizeof(command_id),
        "%s",
        runtime->pending_operation_command);
    runtime->pending_operation_command[0] = '\0';

    return runtime_execute_registered_command(
        runtime,
        command_id,
        value);
}

/*
 * Provide the runtime execute operation profile operation used by this module and its
 * client applications.
 */
static UmiStatus runtime_execute_operation_profile(
    UmiStudioGtkRuntimeChrome *runtime,
    const UmiStudioGtkOperationProfile *profile)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || profile == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Apply this operation only while the related capability or state is available. */
    if (!runtime_registered_command_enabled(
            runtime,
            profile->command_id,
            NULL) &&
        profile->argument_kind == UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE) {
        runtime_show_notification(
            runtime,
            "This Studio operation is currently disabled.");
        return UMI_STATUS_INVALID_STATE;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (profile->argument_kind == UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE ||
        profile->argument_kind == UMI_STUDIO_GTK_OPERATION_ARGUMENT_OPTIONAL) {
        return runtime_execute_registered_command(
            runtime,
            profile->command_id,
            NULL);
    }

    runtime_copy_selection(
        runtime->pending_operation_command,
        sizeof(runtime->pending_operation_command),
        profile->command_id);

    return runtime_prompt_text(
        runtime,
        profile->title,
        runtime_operation_argument_message(profile->argument_kind),
        "",
        "Execute",
        runtime_pending_operation_accepted);
}

/*
 * Provide the runtime operation matches query operation used by this module and its client
 * applications.
 */
static int runtime_operation_matches_query(
    const UmiStudioGtkOperationProfile *profile,
    const char *query)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (profile == NULL) return 0;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (query == NULL || query[0] == '\0') return 1;

    return runtime_text_contains_casefold(profile->title, query) ||
        runtime_text_contains_casefold(profile->category, query) ||
        runtime_text_contains_casefold(profile->description, query) ||
        runtime_text_contains_casefold(profile->command_id, query);
}
