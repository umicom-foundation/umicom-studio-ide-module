/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/debug_details.inc
 *
 * PURPOSE:
 *   Render professional Threads, Call Stack, Variables, Watches and Breakpoints
 *   directly from the Framework UmiDebugWorkspace typed snapshot APIs.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static UmiDebugWorkspace *runtime_debug_workspace(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;
    UmiStudioDebuggerService *debugger;

    if (runtime == NULL || runtime->owner == NULL) return NULL;

    services = umi_studio_ui_services(runtime->owner->ui);
    debugger = services != NULL
        ? umi_studio_services_debugger(services)
        : NULL;

    return debugger != NULL
        ? umi_studio_debugger_service_workspace(debugger)
        : NULL;
}

static void runtime_debug_open_source_uri(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *uri)
{
    GFile *file;
    char *path = NULL;

    if (runtime == NULL || uri == NULL || uri[0] == '\0') return;

    if (g_str_has_prefix(uri, "file:")) {
        file = g_file_new_for_uri(uri);
        if (file != NULL) {
            path = g_file_get_path(file);
            g_object_unref(file);
        }
    } else {
        path = g_strdup(uri);
    }

    if (path != NULL) {
        runtime_explorer_open_path(runtime, path);
        g_free(path);
    }
}

static void runtime_on_debug_thread_row(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *thread_id =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-debug-thread");

    if (runtime == NULL || thread_id == NULL) return;

    (void)runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_DEBUG_SELECT_THREAD,
        thread_id);
}

static void runtime_on_debug_frame_row(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *frame_id =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-debug-frame");
    const char *source_uri =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-debug-source");
    const guint source_line = GPOINTER_TO_UINT(
        g_object_get_data(
            G_OBJECT(button),
            "umicom-debug-line"));

    if (runtime == NULL || frame_id == NULL) return;

    if (runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_DEBUG_SELECT_FRAME,
            frame_id) == UMI_STATUS_OK &&
        source_uri != NULL &&
        source_uri[0] != '\0') {
        runtime_debug_open_source_uri(runtime, source_uri);

        if (source_line > 0U) {
            size_t offset = 0U;
            (void)umi_document_coordinator_go_to_line(
                runtime_documents(runtime),
                (size_t)source_line,
                &offset);
        }
    }
}

static void runtime_debug_details_refresh(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiDebugWorkspace *workspace;
    UmiDebugWorkspaceSnapshot snapshot;
    size_t index;

    if (runtime == NULL) return;

    workspace = runtime_debug_workspace(runtime);
    if (workspace == NULL) return;

    (void)umi_debug_workspace_refresh(workspace);
    (void)memset(&snapshot, 0, sizeof(snapshot));
    if (umi_debug_workspace_snapshot(
            workspace,
            &snapshot) != UMI_STATUS_OK) {
        return;
    }

    runtime_clear_list_box(runtime->debug_threads_list);
    runtime_clear_list_box(runtime->debug_frames_list);
    runtime_clear_list_box(runtime->debug_variables_list);
    runtime_clear_list_box(runtime->debug_watches_list);
    runtime_clear_list_box(runtime->debug_breakpoints_list);

    if (runtime->debug_threads_list != NULL) {
        for (index = 0U; index < snapshot.thread_count; ++index) {
            UmiDebugThreadSnapshot thread;
            GtkWidget *button;
            char text[640];

            (void)memset(&thread, 0, sizeof(thread));
            if (umi_debug_workspace_thread_at(
                    workspace,
                    index,
                    &thread) != UMI_STATUS_OK) {
                continue;
            }

            (void)snprintf(
                text,
                sizeof(text),
                "%s%s%s — %s",
                thread.current ? "▶ " : "",
                thread.stopped ? "■ " : "",
                thread.name,
                thread.detail);

            button = gtk_button_new_with_label(text);
            if (button == NULL) continue;
            gtk_widget_add_css_class(button, "flat");
            g_object_set_data_full(
                G_OBJECT(button),
                "umicom-debug-thread",
                g_strdup(thread.id),
                g_free);
            g_signal_connect(
                button,
                "clicked",
                G_CALLBACK(runtime_on_debug_thread_row),
                runtime);
            gtk_list_box_append(
                GTK_LIST_BOX(runtime->debug_threads_list),
                button);
        }
    }

    if (runtime->debug_frames_list != NULL) {
        for (index = 0U; index < snapshot.visible_frame_count; ++index) {
            UmiDebugStackFrameSnapshot frame;
            GtkWidget *button;
            char text[1400];

            (void)memset(&frame, 0, sizeof(frame));
            if (umi_debug_workspace_frame_at(
                    workspace,
                    index,
                    &frame) != UMI_STATUS_OK) {
                continue;
            }

            (void)snprintf(
                text,
                sizeof(text),
                "%s — %s:%u:%u",
                frame.name,
                frame.source_uri,
                frame.line,
                frame.column);

            button = gtk_button_new_with_label(text);
            if (button == NULL) continue;
            gtk_widget_add_css_class(button, "flat");
            g_object_set_data_full(
                G_OBJECT(button),
                "umicom-debug-frame",
                g_strdup(frame.id),
                g_free);
            g_object_set_data_full(
                G_OBJECT(button),
                "umicom-debug-source",
                g_strdup(frame.source_uri),
                g_free);
            g_object_set_data(
                G_OBJECT(button),
                "umicom-debug-line",
                GUINT_TO_POINTER(frame.line));
            g_signal_connect(
                button,
                "clicked",
                G_CALLBACK(runtime_on_debug_frame_row),
                runtime);
            gtk_list_box_append(
                GTK_LIST_BOX(runtime->debug_frames_list),
                button);
        }
    }

    if (runtime->debug_variables_list != NULL) {
        for (index = 0U; index < snapshot.visible_variable_count; ++index) {
            UmiDebugVariableSnapshot variable;
            char text[1700];

            (void)memset(&variable, 0, sizeof(variable));
            if (umi_debug_workspace_variable_at(
                    workspace,
                    index,
                    &variable) != UMI_STATUS_OK) {
                continue;
            }

            (void)snprintf(
                text,
                sizeof(text),
                "%s%s = %s  [%s]",
                variable.changed ? "• " : "",
                variable.name,
                variable.value,
                variable.type);
            gtk_list_box_append(
                GTK_LIST_BOX(runtime->debug_variables_list),
                runtime_label(text, 0.0F));
        }
    }

    if (runtime->debug_watches_list != NULL) {
        for (index = 0U; index < snapshot.watch_count; ++index) {
            UmiDebugWatchSnapshot watch;
            char text[1800];

            (void)memset(&watch, 0, sizeof(watch));
            if (umi_debug_workspace_watch_at(
                    workspace,
                    index,
                    &watch) != UMI_STATUS_OK) {
                continue;
            }

            (void)snprintf(
                text,
                sizeof(text),
                "%s = %s  [%s]%s",
                watch.expression,
                watch.value,
                watch.type,
                watch.valid ? "" : "  invalid");
            gtk_list_box_append(
                GTK_LIST_BOX(runtime->debug_watches_list),
                runtime_label(text, 0.0F));
        }
    }

    if (runtime->debug_breakpoints_list != NULL) {
        for (index = 0U; index < snapshot.breakpoint_count; ++index) {
            UmiDebugBreakpointSnapshot breakpoint;
            char text[1500];

            (void)memset(&breakpoint, 0, sizeof(breakpoint));
            if (umi_debug_workspace_breakpoint_at(
                    workspace,
                    index,
                    &breakpoint) != UMI_STATUS_OK) {
                continue;
            }

            (void)snprintf(
                text,
                sizeof(text),
                "%s%s — %s:%u:%u%s",
                breakpoint.enabled ? "● " : "○ ",
                breakpoint.verified ? "" : "? ",
                breakpoint.uri,
                breakpoint.line,
                breakpoint.column,
                breakpoint.condition[0] != '\0'
                    ? " — conditional"
                    : "");
            gtk_list_box_append(
                GTK_LIST_BOX(runtime->debug_breakpoints_list),
                runtime_label(text, 0.0F));
        }
    }
}

static GtkWidget *runtime_debug_details_notebook(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *notebook;

    if (runtime == NULL) return NULL;

    notebook = gtk_notebook_new();
    runtime->debug_threads_list = gtk_list_box_new();
    runtime->debug_frames_list = gtk_list_box_new();
    runtime->debug_variables_list = gtk_list_box_new();
    runtime->debug_watches_list = gtk_list_box_new();
    runtime->debug_breakpoints_list = gtk_list_box_new();

    if (notebook == NULL ||
        runtime->debug_threads_list == NULL ||
        runtime->debug_frames_list == NULL ||
        runtime->debug_variables_list == NULL ||
        runtime->debug_watches_list == NULL ||
        runtime->debug_breakpoints_list == NULL) {
        return notebook;
    }

    gtk_widget_set_size_request(notebook, -1, 250);

    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        runtime_scroll(runtime->debug_threads_list),
        gtk_label_new("Threads"));
    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        runtime_scroll(runtime->debug_frames_list),
        gtk_label_new("Call Stack"));
    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        runtime_scroll(runtime->debug_variables_list),
        gtk_label_new("Variables"));
    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        runtime_scroll(runtime->debug_watches_list),
        gtk_label_new("Watches"));
    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook),
        runtime_scroll(runtime->debug_breakpoints_list),
        gtk_label_new("Breakpoints"));

    runtime_debug_details_refresh(runtime);
    return notebook;
}
