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

static UmiStudioTerminalService *runtime_terminal_service(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;

    if (runtime == NULL || runtime->owner == NULL) return NULL;
    services = umi_studio_ui_services(runtime->owner->ui);
    return services != NULL
        ? umi_studio_services_terminal(services)
        : NULL;
}

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

    if (runtime == NULL || runtime->terminal_text_view == NULL) return;

    service = runtime_terminal_service(runtime);
    session = service != NULL
        ? umi_studio_terminal_service_primary(service)
        : NULL;
    transcript = session != NULL
        ? umi_terminal_session_transcript(session)
        : NULL;

    text = g_string_sized_new(4096U);
    if (text == NULL) return;

    count = transcript != NULL
        ? umi_terminal_transcript_count(transcript)
        : 0U;
    start = count > UMI_STUDIO_GTK_TRANSCRIPT_ROW_LIMIT
        ? count - UMI_STUDIO_GTK_TRANSCRIPT_ROW_LIMIT
        : 0U;

    for (index = start; index < count; ++index) {
        UmiTerminalTranscriptLine line;
        const char *prefix = "";

        (void)memset(&line, 0, sizeof(line));
        if (umi_terminal_transcript_at(
                transcript,
                index,
                &line) != UMI_STATUS_OK) {
            continue;
        }

        switch (line.stream) {
            case UMI_TERMINAL_STREAM_INPUT: prefix = "> "; break;
            case UMI_TERMINAL_STREAM_ERROR: prefix = "! "; break;
            case UMI_TERMINAL_STREAM_SYSTEM: prefix = "# "; break;
            case UMI_TERMINAL_STREAM_OUTPUT:
            default: prefix = ""; break;
        }

        g_string_append(text, prefix);
        g_string_append(text, line.text);
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

    if (runtime->terminal_summary_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(runtime->terminal_summary_label),
            summary);
    }

    g_string_free(text, TRUE);
}

static void runtime_terminal_execute_entry(
    UmiStudioGtkRuntimeChrome *runtime)
{
    const char *command;

    if (runtime == NULL || runtime->terminal_entry == NULL) return;

    command = gtk_editable_get_text(
        GTK_EDITABLE(runtime->terminal_entry));
    if (command == NULL || command[0] == '\0') return;

    runtime_copy_selection(
        runtime->last_terminal_command,
        sizeof(runtime->last_terminal_command),
        command);

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

static void runtime_on_terminal_entry_activate(
    GtkEntry *entry,
    gpointer user_data)
{
    (void)entry;
    runtime_terminal_execute_entry(
        (UmiStudioGtkRuntimeChrome *)user_data);
}

static void runtime_on_terminal_run(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_terminal_execute_entry(
        (UmiStudioGtkRuntimeChrome *)user_data);
}


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

static void runtime_on_terminal_clear(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    (void)button;

    if (runtime == NULL) return;

    (void)runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_TERMINAL_CLEAR,
        NULL);
    runtime_terminal_refresh_transcript(runtime);
}
