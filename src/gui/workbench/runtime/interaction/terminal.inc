/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/terminal.inc
 *
 * PURPOSE:
 *   Present the Framework terminal transcript and execute entered commands via
 *   the canonical Studio command registry.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#include "umicom/terminal_ui/execution.h"

/* A retained control resolves the current workbench through a weak binding.
 * It cannot launch or stop work after the window's runtime is detached. */
static void RuntimeTerminalBindingFree(gpointer context, GClosure *closure)
{
    (void)closure;
    runtime_file_dialog_request_free(context);
}
static void RuntimeTerminalBind(GtkWidget *widget, const char *signal,
    GCallback callback, UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioFileDialogRequest *binding = runtime_file_dialog_request_new(runtime, 0U);
    if (binding == NULL) { gtk_widget_set_sensitive(widget, FALSE); return; }
    g_signal_connect_data(widget, signal, callback, binding, RuntimeTerminalBindingFree, 0);
}
static UmiTerminalController *RuntimeTerminalController(UmiStudioGtkRuntimeChrome *runtime)
{
    return runtime != NULL && runtime->owner != NULL && runtime->owner->ui != NULL
        ? umi_studio_services_terminal_controller(umi_studio_ui_services(runtime->owner->ui))
        : NULL;
}
/* Poll even when Terminal is hidden, so a completed command releases capacity
 * and project switching does not depend on which panel is visible. */
static void RuntimeTerminalPoll(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiTerminalController *controller = RuntimeTerminalController(runtime);
    if (!UmiTerminalControllerJobPending(controller)) return;
    UmiStudioWorkspaceSnapshot workspace;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    if (umi_studio_workspace_snapshot(services, &workspace) != UMI_STATUS_OK ||
        !workspace.graph.open || !workspace.graph.trusted)
        (void)UmiTerminalControllerStopJob(controller);
    UmiTerminalControllerExecution result;
    (void)UmiTerminalControllerPollJob(controller, &result);
}
static void RuntimeTerminalStop(GtkButton *button, gpointer context)
{
    (void)button;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &window);
    if (runtime != NULL)
        (void)runtime_execute_registered_command(runtime, UMI_STUDIO_COMMAND_TERMINAL_STOP, NULL);
    g_clear_object(&window);
}

static UmiStudioTerminalService *runtime_terminal_service(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL) return NULL;
    services = umi_studio_ui_services(runtime->owner->ui);
    return services != NULL
        ? umi_studio_services_terminal(services)
        : NULL;
}

/*
 * Provide the runtime terminal refresh transcript operation used by this module and its
 * client applications.
 */
/* The active terminal and its background job now determine the displayed transcript. The primary-session-only renderer is retained for review. The previous implementation is retained for engineering review. */
#if 0
static void runtime_terminal_refresh_transcript(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioTerminalService *service;
    UmiTerminalSession *session;
    UmiTerminalTranscript *transcript;
    GtkTextBuffer *buffer;
    GString *text;
    size_t count;
    size_t start;
    size_t index;
    char summary[256];

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->terminal_text_view == NULL) return;

    service = runtime_terminal_service(runtime);
    session = service != NULL
        ? umi_studio_terminal_service_primary(service)
        : NULL;
    transcript = session != NULL
        ? umi_terminal_session_transcript(session)
        : NULL;

    text = g_string_sized_new(4096U);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (text == NULL) return;

    count = transcript != NULL
        ? umi_terminal_transcript_count(transcript)
        : 0U;
    start = count > UMI_STUDIO_GTK_TRANSCRIPT_ROW_LIMIT
        ? count - UMI_STUDIO_GTK_TRANSCRIPT_ROW_LIMIT
        : 0U;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = start; index < count; ++index) {
        UmiTerminalTranscriptLine line;
        const char *prefix = "";

        (void)memset(&line, 0, sizeof(line));
        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_terminal_transcript_at(
                transcript,
                index,
                &line) != UMI_STATUS_OK) {
            continue;
        }

        /* Select the behaviour associated with the requested command or state value. */
        switch (line.stream) {
            case UMI_TERMINAL_STREAM_INPUT: prefix = "> "; break;
            case UMI_TERMINAL_STREAM_ERROR: prefix = "! "; break;
            case UMI_TERMINAL_STREAM_SYSTEM: prefix = "# "; break;
            case UMI_TERMINAL_STREAM_OUTPUT:
            default: prefix = ""; break;
        }

        g_string_append(text, prefix);
        g_string_append(text, line.text);
        /* Apply this branch only when its contract condition is satisfied. */
        if (text->len == 0U ||
            text->str[text->len - 1U] != '\n') {
            g_string_append_c(text, '\n');
        }
    }

    buffer = gtk_text_view_get_buffer(
        GTK_TEXT_VIEW(runtime->terminal_text_view));
    gtk_text_buffer_set_text(buffer, text->str, -1);

    (void)snprintf(
        summary,
        sizeof(summary),
        "%zu session%s — %zu transcript line%s — last exit %d",
        runtime->terminal_snapshot.session_count,
        runtime->terminal_snapshot.session_count == 1U ? "" : "s",
        count,
        count == 1U ? "" : "s",
        runtime->terminal_snapshot.primary.last_exit_code);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->terminal_summary_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(runtime->terminal_summary_label),
            summary);
    }

    g_string_free(text, TRUE);
}
#endif
static void runtime_terminal_refresh_transcript(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->terminal_text_view == NULL) return;
    UmiTerminalController *controller = RuntimeTerminalController(runtime);
    UmiTerminalSession *session = umi_terminal_controller_active_session(controller);
    if (session == NULL) {
        UmiStudioTerminalService *service = runtime_terminal_service(runtime);
        session = service != NULL ? umi_studio_terminal_service_primary(service) : NULL;
    }
    UmiTerminalSessionSnapshot selected = {0};
    if (session != NULL) (void)umi_terminal_session_snapshot(session, &selected);
    /* A different selected tab gets its own folder. Do not erase a draft path
     * on ordinary refreshes while the user is typing. */
    if (runtime->terminal_directory_entry != NULL &&
        strcmp(runtime->terminal_directory_session, selected.session_id) != 0) {
        memcpy(runtime->terminal_directory_session, selected.session_id, strlen(selected.session_id) + 1U);
        gtk_editable_set_text(GTK_EDITABLE(runtime->terminal_directory_entry), selected.working_directory);
    }
    UmiTerminalTranscript *transcript = session != NULL
        ? umi_terminal_session_transcript(session) : NULL;
    UmiTerminalControllerExecution running = {0};
    (void)UmiTerminalControllerPollJob(controller, &running);
    GString *text = g_string_sized_new(4096U);
    if (text == NULL) return;
    size_t count = transcript != NULL ? umi_terminal_transcript_count(transcript) : 0U;
    size_t start = count > UMI_STUDIO_GTK_TRANSCRIPT_ROW_LIMIT
        ? count - UMI_STUDIO_GTK_TRANSCRIPT_ROW_LIMIT : 0U;
    for (size_t index = start; index < count; ++index) {
        UmiTerminalTranscriptLine line;
        if (umi_terminal_transcript_at(transcript, index, &line) != UMI_STATUS_OK) continue;
        const char *prefix = line.stream == UMI_TERMINAL_STREAM_INPUT ? "> " :
            line.stream == UMI_TERMINAL_STREAM_ERROR ? "! " :
            line.stream == UMI_TERMINAL_STREAM_SYSTEM ? "# " : "";
        g_string_append(text, prefix);
        g_string_append(text, line.text);
        if (text->len == 0U || text->str[text->len - 1U] != '\n') g_string_append_c(text, '\n');
    }
    /* The pending tail belongs only to its captured session. Switching tabs
     * changes the transcript view, never the identity or destination of output. */
    if (running.execution.pending &&
        strcmp(running.session_id, selected.session_id) == 0) {
        g_string_append(text, "\n--- Running command output ---\n");
        if (running.execution.process.output_truncated)
            g_string_append(text, "[Earlier output omitted from this bounded preview]\n");
        g_string_append(text, running.execution.process.output);
    }
    char *valid = g_utf8_make_valid(text->str, (gssize)text->len);
    const char *previous = g_object_get_data(G_OBJECT(runtime->terminal_text_view), "umicom-terminal-rendered");
    /* Do not rewrite an unchanged buffer on every timer tick; that would discard
     * the user's selection while they are reading or copying a diagnostic. */
    if (previous == NULL || strcmp(previous, valid) != 0) {
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(runtime->terminal_text_view)), valid, -1);
        g_object_set_data_full(G_OBJECT(runtime->terminal_text_view), "umicom-terminal-rendered",
            g_strdup(valid), g_free);
    }
    g_free(valid);
    g_string_free(text, TRUE);
    char *summary = running.execution.pending
        ? g_strdup_printf("%s — %s — %zu transcript records — %s command in %s",
            selected.title, selected.working_directory, count, running.execution.stop_requested ? "Stopping" : "Running",
            running.session_id)
        : g_strdup_printf("%s — %s — %zu transcript records — last exit %d",
            selected.title, selected.working_directory, count, selected.last_exit_code);
    char *safe_summary = g_utf8_make_valid(summary, -1);
    if (runtime->terminal_summary_label != NULL)
        gtk_label_set_text(GTK_LABEL(runtime->terminal_summary_label), safe_summary);
    g_free(safe_summary);
    g_free(summary);
    if (runtime->terminal_run_button != NULL)
        gtk_widget_set_sensitive(runtime->terminal_run_button, !running.execution.pending);
    if (runtime->terminal_stop_button != NULL)
        gtk_widget_set_sensitive(runtime->terminal_stop_button, running.execution.pending);
}

/*
 * Provide the runtime terminal execute entry operation used by this module and its client
 * applications.
 */
/* Command text and a weak owner binding now survive registry callbacks. The old direct-widget route is retained for review. The previous implementation is retained for engineering review. */
#if 0
static void runtime_terminal_execute_entry(
    UmiStudioGtkRuntimeChrome *runtime)
{
    const char *command;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->terminal_entry == NULL) return;

    command = gtk_editable_get_text(
        GTK_EDITABLE(runtime->terminal_entry));
    /* Use the shared build helper when it is available from the parent composition. */
    if (command == NULL || command[0] == '\0') return;

    runtime_copy_selection(
        runtime->last_terminal_command,
        sizeof(runtime->last_terminal_command),
        command);

    /* Apply this branch only when its contract condition is satisfied. */
    if (runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_TERMINAL_EXECUTE,
            command) == UMI_STATUS_OK) {
        gtk_editable_set_text(
            GTK_EDITABLE(runtime->terminal_entry),
            "");
    }

    runtime_terminal_refresh_transcript(runtime);
}
#endif
static void runtime_terminal_execute_entry(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->terminal_entry == NULL) return;
    char *command = g_strdup(gtk_editable_get_text(GTK_EDITABLE(runtime->terminal_entry)));
    if (command == NULL || command[0] == '\0') { g_free(command); return; }
    UmiStudioFileDialogRequest *binding = runtime_file_dialog_request_new(runtime, 0U);
    if (binding == NULL) { g_free(command); return; }
    runtime_copy_selection(runtime->last_terminal_command, sizeof runtime->last_terminal_command, command);
    UmiStatus status = runtime_execute_registered_command(runtime, UMI_STUDIO_COMMAND_TERMINAL_EXECUTE, command);
    /* Command notifications may detach the workbench. Resolve its binding again
     * before clearing an entry or refreshing any widgets. */
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(binding, &window);
    if (runtime != NULL) {
        if (status == UMI_STATUS_OK && runtime->terminal_entry != NULL)
            gtk_editable_set_text(GTK_EDITABLE(runtime->terminal_entry), "");
        runtime_terminal_refresh_transcript(runtime);
    }
    g_clear_object(&window);
    runtime_file_dialog_request_free(binding);
    g_free(command);
}

/*
 * Provide the runtime on terminal entry activate operation used by this module and its
 * client applications.
 */
/* Terminal controls now use a weak workbench binding so retained widgets cannot operate on a closed runtime. The previous callback is retained for review. The previous implementation is retained for engineering review. */
#if 0
static void runtime_on_terminal_entry_activate(
    GtkEntry *entry,
    gpointer user_data)
{
    (void)entry;
    runtime_terminal_execute_entry(
        (UmiStudioGtkRuntimeChrome *)user_data);
}
#endif
static void runtime_on_terminal_entry_activate(GtkEntry *entry, gpointer context)
{
    (void)entry;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &window);
    if (runtime != NULL) runtime_terminal_execute_entry(runtime);
    g_clear_object(&window);
}

/*
 * Perform runtime on terminal through the module contract so client applications do not
 * duplicate its policy.
 */
/* Terminal controls now use a weak workbench binding so retained widgets cannot operate on a closed runtime. The previous callback is retained for review. The previous implementation is retained for engineering review. */
#if 0
static void runtime_on_terminal_run(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_terminal_execute_entry(
        (UmiStudioGtkRuntimeChrome *)user_data);
}
#endif
static void runtime_on_terminal_run(GtkButton *button, gpointer context)
{
    (void)button;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &window);
    if (runtime != NULL) runtime_terminal_execute_entry(runtime);
    g_clear_object(&window);
}


/*
 * Provide the runtime on terminal new operation used by this module and its client
 * applications.
 */
/* Terminal controls now use a weak workbench binding so retained widgets cannot operate on a closed runtime. The previous callback is retained for review. The previous implementation is retained for engineering review. */
#if 0
static void runtime_on_terminal_new(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    (void)runtime_execute_registered_command(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_TERMINAL_NEW,
        NULL);
}
#endif
static void runtime_on_terminal_new(GtkButton *button, gpointer context)
{
    (void)button;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &window);
    if (runtime != NULL) (void)runtime_execute_registered_command(runtime, UMI_STUDIO_COMMAND_TERMINAL_NEW, NULL);
    g_clear_object(&window);
}

/*
 * Release or reset state held by runtime on terminal so the same storage can be reused
 * safely.
 */
/* Terminal controls now use a weak workbench binding so retained widgets cannot operate on a closed runtime. The previous callback is retained for review. The previous implementation is retained for engineering review. */
#if 0
static void runtime_on_terminal_clear(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    (void)button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;

    (void)runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_TERMINAL_CLEAR,
        NULL);
    runtime_terminal_refresh_transcript(runtime);
}
#endif
static void runtime_on_terminal_clear(GtkButton *button, gpointer context)
{
    (void)button;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &window);
    if (runtime != NULL) (void)runtime_execute_registered_command(runtime, UMI_STUDIO_COMMAND_TERMINAL_CLEAR, NULL);
    g_clear_object(&window);
}

/* Explicit folder selection shares the weak bindings and command dispatch above. */
#include "terminal_location.inc"

/* Programs that need stdin use a separate Framework-owned console. */
#include "program_console.inc"
