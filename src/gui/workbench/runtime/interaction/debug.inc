/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/debug.inc
 *
 * PURPOSE:
 *   Present operational debugger controls over the existing Studio DAP service.
 *   Thread commands are routed through the canonical command registry.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_execute_command(UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id, const char *argument);

/* Adapter configuration is copied by the Studio host; Framework owns profiles
 * and starts the executable without a command shell. */
static void RuntimeDebugConfigure(GtkButton *button, gpointer context)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = context;
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL ||
        runtime->debug_adapter_kind == NULL || runtime->debug_adapter_executable == NULL) return;
    guint selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(runtime->debug_adapter_kind));
    const char *path = gtk_editable_get_text(GTK_EDITABLE(runtime->debug_adapter_executable));
    UmiStatus status = UmiStudioDebuggerConfigureNative(
        umi_studio_services_debugger(umi_studio_ui_services(runtime->owner->ui)),
        selected == 0U ? "gdb" : "lldb", path);
    runtime_show_notification(runtime, status == UMI_STATUS_OK
        ? "Native debugger selected. Debug builds the project before starting it."
        : umi_status_text(status));
}

static const char *runtime_debug_thread_argument(
    UmiStudioGtkRuntimeChrome *runtime)
{
    const char *value;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->debug_thread_entry == NULL) {
        return "0";
    }

    value = gtk_editable_get_text(
        GTK_EDITABLE(runtime->debug_thread_entry));
    return value != NULL && value[0] != '\0' ? value : "0";
}

/*
 * Provide the runtime debug command operation used by this module and its client
 * applications.
 */
static void runtime_debug_command(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    int thread_scoped)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || command_id == NULL) return;

    (void)runtime_execute_registered_command(
        runtime,
        command_id,
        thread_scoped
            ? runtime_debug_thread_argument(runtime)
            : NULL);
}

/*
 * Provide the runtime on debug start operation used by this module and its client
 * applications.
 */
static void runtime_on_debug_start(GtkButton *button, gpointer user_data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    if (runtime == NULL || runtime->owner == NULL) return;
    /* Use the same saved-source gate as the Debug menu and command palette. */
    (void)runtime_execute_command(runtime, "debug.start", NULL);
}

/*
 * Provide the runtime on debug continue operation used by this module and its client
 * applications.
 */
static void runtime_on_debug_continue(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_CONTINUE,
        1);
}

/*
 * Provide the runtime on debug pause operation used by this module and its client
 * applications.
 */
static void runtime_on_debug_pause(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_PAUSE,
        1);
}

/*
 * Provide the runtime on debug next operation used by this module and its client
 * applications.
 */
static void runtime_on_debug_next(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_NEXT,
        1);
}

/*
 * Provide the runtime on debug step in operation used by this module and its client
 * applications.
 */
static void runtime_on_debug_step_in(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_STEP_IN,
        1);
}

/*
 * Provide the runtime on debug step out operation used by this module and its client
 * applications.
 */
static void runtime_on_debug_step_out(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_STEP_OUT,
        1);
}

/*
 * Provide the runtime on debug stop operation used by this module and its client
 * applications.
 */
static void runtime_on_debug_stop(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_STOP,
        0);
}

/*
 * Provide the runtime debug watch accepted operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_debug_watch_accepted(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *expression)
{
    return runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_DEBUG_ADD_WATCH,
        expression);
}

/*
 * Provide the runtime on debug add watch operation used by this module and its client
 * applications.
 */
static void runtime_on_debug_add_watch(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    (void)runtime_prompt_text(
        (UmiStudioGtkRuntimeChrome *)user_data,
        "Add Watch",
        "Enter a debugger expression.",
        "",
        "Add",
        runtime_debug_watch_accepted);
}

/*
 * Provide the runtime debug update summary operation used by this module and its client
 * applications.
 */
static void runtime_debug_update_summary(
    UmiStudioGtkRuntimeChrome *runtime)
{
    char text[256];

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->debug_summary_label == NULL) return;

    (void)snprintf(
        text,
        sizeof(text),
        "%s — %zu session%s, %zu thread%s, %zu frame%s, "
        "%zu breakpoint%s, %zu watch%s",
        runtime->debugger_snapshot.controller_state[0] != '\0'
            ? runtime->debugger_snapshot.controller_state
            : "idle",
        runtime->debugger_snapshot.session_count,
        runtime->debugger_snapshot.session_count == 1U ? "" : "s",
        runtime->debugger_snapshot.thread_count,
        runtime->debugger_snapshot.thread_count == 1U ? "" : "s",
        runtime->debugger_snapshot.stack_frame_count,
        runtime->debugger_snapshot.stack_frame_count == 1U ? "" : "s",
        runtime->debugger_snapshot.breakpoint_count,
        runtime->debugger_snapshot.breakpoint_count == 1U ? "" : "s",
        runtime->debugger_snapshot.watch_count,
        runtime->debugger_snapshot.watch_count == 1U ? "" : "es");

    gtk_label_set_text(
        GTK_LABEL(runtime->debug_summary_label),
        text);
}

// MIGRATION REFERENCE — previous implementation excerpts
// Native execution now delegates to Framework debug_runtime/platform.c and build/project_session.c. Studio retains command composition and presentation; the explicit legacy memory-protocol API remains available for its existing callers.
// These comments explain superseded statements; do not enable both execution paths.
// Previous source near line 65:
//     runtime_debug_command(
//         (UmiStudioGtkRuntimeChrome *)user_data,
//         UMI_STUDIO_COMMAND_DEBUG_START,
//         0);
