/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/debug.inc
 *
 * PURPOSE:
 *   Present operational debugger controls over the existing Studio DAP service.
 *   Thread commands are routed through the canonical command registry.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static const char *runtime_debug_thread_argument(
    UmiStudioGtkRuntimeChrome *runtime)
{
    const char *value;

    if (runtime == NULL || runtime->debug_thread_entry == NULL) {
        return "0";
    }

    value = gtk_editable_get_text(
        GTK_EDITABLE(runtime->debug_thread_entry));
    return value != NULL && value[0] != '\0' ? value : "0";
}

static void runtime_debug_command(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    int thread_scoped)
{
    if (runtime == NULL || command_id == NULL) return;

    (void)runtime_execute_registered_command(
        runtime,
        command_id,
        thread_scoped
            ? runtime_debug_thread_argument(runtime)
            : NULL);
}

static void runtime_on_debug_start(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_START,
        0);
}

static void runtime_on_debug_continue(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_CONTINUE,
        1);
}

static void runtime_on_debug_pause(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_PAUSE,
        1);
}

static void runtime_on_debug_next(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_NEXT,
        1);
}

static void runtime_on_debug_step_in(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_STEP_IN,
        1);
}

static void runtime_on_debug_step_out(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_STEP_OUT,
        1);
}

static void runtime_on_debug_stop(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_debug_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_DEBUG_STOP,
        0);
}

static UmiStatus runtime_debug_watch_accepted(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *expression)
{
    return runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_DEBUG_ADD_WATCH,
        expression);
}

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

static void runtime_debug_update_summary(
    UmiStudioGtkRuntimeChrome *runtime)
{
    char text[256];

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
