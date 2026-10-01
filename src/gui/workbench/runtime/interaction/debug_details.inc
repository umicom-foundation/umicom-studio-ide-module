/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/debug_details.inc
 *
 * PURPOSE:
 *   Render professional Threads, Call Stack, Variables, Watches and Breakpoints
 *   directly from the Framework UmiDebugWorkspace typed snapshot APIs.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static UmiDebugWorkspace *runtime_debug_workspace(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;
    UmiStudioDebuggerService *debugger;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL) return NULL;

    services = umi_studio_ui_services(runtime->owner->ui);
    debugger = services != NULL
        ? umi_studio_services_debugger(services)
        : NULL;

    return debugger != NULL
        ? umi_studio_debugger_service_workspace(debugger)
        : NULL;
}

/*
 * Provide the runtime debug open source uri operation used by this module and its client
 * applications.
 */
/* Raw debugger callbacks and unchecked source opens are superseded by owned Framework selection evidence, weak window actions and checked document navigation. This prevents stale IDs, retained controls and failed opens from targeting the wrong editor. The previous implementation remains for engineering review. */
#if 0
static void runtime_debug_open_source_uri(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *uri)
{
    GFile *file;
    char *path = NULL;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || uri == NULL || uri[0] == '\0') return;

    /* Apply this branch only when its contract condition is satisfied. */
    if (g_str_has_prefix(uri, "file:")) {
        file = g_file_new_for_uri(uri);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (file != NULL) {
            path = g_file_get_path(file);
            g_object_unref(file);
        }
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        path = g_strdup(uri);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (path != NULL) {
        runtime_explorer_open_path(runtime, path);
        g_free(path);
    }
}

/*
 * Provide the runtime on debug thread row operation used by this module and its client
 * applications.
 */
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

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || thread_id == NULL) return;

    (void)runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_DEBUG_SELECT_THREAD,
        thread_id);
}

/*
 * Provide the runtime on debug frame row operation used by this module and its client
 * applications.
 */
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

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || frame_id == NULL) return;

    /* Apply this branch only when its contract condition is satisfied. */
    if (runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_DEBUG_SELECT_FRAME,
            frame_id) == UMI_STATUS_OK &&
        source_uri != NULL &&
        source_uri[0] != '\0') {
        runtime_debug_open_source_uri(runtime, source_uri);

        /* Apply this branch only when its contract condition is satisfied. */
        if (source_line > 0U) {
            size_t offset = 0U;
            (void)umi_document_coordinator_go_to_line(
                runtime_documents(runtime),
                (size_t)source_line,
                &offset);
        }
    }
}
#endif
#include "umicom/debug/selection.h"
#include "umicom/debug_ui/navigation.h"

static void RuntimeDebugSelectionDestroy(gpointer data)
{
    UmiDebugSelectionDestroy(data);
}
static void RuntimeDebugRowClicked(GtkButton *button,gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime=data;
    const UmiDebugSelection *selection=g_object_get_data(G_OBJECT(button),"umicom-debug-selection");
    UmiDebugSelectionSnapshot selected;
    UmiStatus status=UmiDebugSelectionValidate(runtime_debug_workspace(runtime),selection,&selected);
    if(status!=UMI_STATUS_OK){
        runtime_show_notification(runtime,"Debugger state changed. Refresh and choose the current thread or frame again.");return;
    }
    /* Copy before dispatch: refreshing services can destroy the clicked row.
     * The registered command retains its usual trust, history and DAP policy. */
    status=runtime_execute_registered_command(runtime,
        selected.kind==UMI_DEBUG_SELECT_THREAD?UMI_STUDIO_COMMAND_DEBUG_SELECT_THREAD:UMI_STUDIO_COMMAND_DEBUG_SELECT_FRAME,
        selected.kind==UMI_DEBUG_SELECT_THREAD?selected.thread.id:selected.frame.id);
    if(status==UMI_STATUS_OK&&selected.kind==UMI_DEBUG_SELECT_FRAME)
        status=UmiDebugFrameOpenSource(runtime_documents(runtime),&selected.frame,selected.sourceBase,NULL);
    (void)runtime_refresh_services(runtime);
    if(status!=UMI_STATUS_OK){
        char message[240];(void)snprintf(message,sizeof(message),
            "Debugger selection or source navigation did not complete: %s. No text was changed.",umi_status_text(status));
        runtime_show_notification(runtime,message);
    }
}
static void RuntimeDebugBindRow(UmiStudioGtkRuntimeChrome *runtime,GtkWidget *button,
    UmiDebugWorkspace *workspace,UmiDebugSelectionKind kind,size_t index)
{
    UmiDebugSelection *selection=NULL;
    UmiStatus status=UmiDebugSelectionCapture(workspace,kind,index,&selection);
    g_object_set_data_full(G_OBJECT(button),"umicom-debug-selection",selection,RuntimeDebugSelectionDestroy);
    char tag[80];(void)snprintf(tag,sizeof(tag),"studio.debug.%s.%zu",kind==UMI_DEBUG_SELECT_THREAD?"thread":"frame",index);
    (void)umi_gtk4_automation_tag_widget(button,tag);
    gtk_widget_set_sensitive(button,status==UMI_STATUS_OK);
    if(status!=UMI_STATUS_OK)gtk_widget_set_tooltip_text(button,"This row has no current stopped-frame target. Refresh the debugger.");
    runtime_bind_editor_action(runtime,button,RuntimeDebugRowClicked);
}


/*
 * Provide the runtime debug details refresh operation used by this module and its client
 * applications.
 */
#include "debug_breakpoints.inc"
#include "debug_watches.inc"
#include "debug_variables.inc"
#include "debug_scopes.inc"

static void runtime_debug_details_refresh(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiDebugWorkspace *workspace;
    UmiDebugWorkspaceSnapshot snapshot;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;

    workspace = runtime_debug_workspace(runtime);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workspace == NULL) return;

    (void)umi_debug_workspace_refresh(workspace);
    (void)memset(&snapshot, 0, sizeof(snapshot));
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_debug_workspace_snapshot(
            workspace,
            &snapshot) != UMI_STATUS_OK) {
        return;
    }

    /* Each model has its own generation. Unchanged timer ticks retain the
     * actual buttons and keyboard focus rather than rebuilding every list. */
    UmiDebugViewStamp stamp;
    if(UmiDebugWorkspaceViewStamp(workspace,&stamp)!=UMI_STATUS_OK)return;
    RuntimeDebugBreakpointsRefresh(runtime, workspace, &stamp);
    RuntimeDebugWatchesRefresh(runtime, workspace, &stamp);
    if(runtime->debug_threads_list!=NULL){
        const UmiDebugViewStamp *previous=g_object_get_data(G_OBJECT(runtime->debug_threads_list),"umicom-debug-view-stamp");
        if(UmiDebugViewStampEqual(previous,&stamp))return;
        UmiDebugViewStamp *stored=g_new(UmiDebugViewStamp,1);*stored=stamp;
        g_object_set_data_full(G_OBJECT(runtime->debug_threads_list),"umicom-debug-view-stamp",stored,g_free);
    }

    runtime_clear_list_box(runtime->debug_threads_list);
    runtime_clear_list_box(runtime->debug_frames_list);
    runtime_clear_list_box(runtime->debug_variables_list);
/* Watch properties now own their collection refresh; avoid discarding unapplied text when variables or frames change. The previous implementation remains for engineering review. */
#if 0
    runtime_clear_list_box(runtime->debug_watches_list);
#endif
    /* The dedicated watch renderer retains drafts across unrelated refreshes. */
/* Breakpoint property fields now retain drafts until their own source collection changes. Their dedicated refresh owns list replacement. The previous implementation remains for engineering review. */
#if 0
    runtime_clear_list_box(runtime->debug_breakpoints_list);
#endif
    /* Breakpoint rows have their own generation; keep unapplied fields during unrelated refreshes. */

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->debug_threads_list != NULL) {
        /* Visit each bounded item once so every record receives the same rule. */
        for (index = 0U; index < snapshot.thread_count; ++index) {
            UmiDebugThreadSnapshot thread;
            GtkWidget *button;
            char text[640];

            (void)memset(&thread, 0, sizeof(thread));
            /* Apply this branch only when its contract condition is satisfied. */
            if (umi_debug_workspace_thread_at(
                    workspace,
                    index,
                    &thread) != UMI_STATUS_OK) {
                continue;
            }

            (void)snprintf(
                text,
                sizeof(text),
                "%.4s%.4s%.250s - %.350s",
                thread.current ? "> " : "",
                thread.stopped ? "# " : "",
                thread.name,
                thread.detail);

            button = gtk_button_new_with_label(text);
            /*
             * Protect caller-owned memory by checking that required state is available before it is
             * used.
             */
            if (button == NULL) continue;
            gtk_widget_add_css_class(button, "flat");
            g_object_set_data_full(
                G_OBJECT(button),
                "umicom-debug-thread",
                g_strdup(thread.id),
                g_free);
/* Rows now own captured targets and use the existing weak window action binding. The previous implementation remains for engineering review. */
#if 0
            g_signal_connect(
                button,
                "clicked",
                G_CALLBACK(runtime_on_debug_thread_row),
                runtime);
#endif
            RuntimeDebugBindRow(runtime,button,workspace,UMI_DEBUG_SELECT_THREAD,index);
            gtk_list_box_append(
                GTK_LIST_BOX(runtime->debug_threads_list),
                button);
        }
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->debug_frames_list != NULL) {
        /* Visit each bounded item once so every record receives the same rule. */
        for (index = 0U; index < snapshot.visible_frame_count; ++index) {
            UmiDebugStackFrameSnapshot frame;
            GtkWidget *button;
            char text[1400];

            (void)memset(&frame, 0, sizeof(frame));
            /* Apply this branch only when its contract condition is satisfied. */
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
            /*
             * Protect caller-owned memory by checking that required state is available before it is
             * used.
             */
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
/* Rows now own captured targets and use the existing weak window action binding. The previous implementation remains for engineering review. */
#if 0
            g_signal_connect(
                button,
                "clicked",
                G_CALLBACK(runtime_on_debug_frame_row),
                runtime);
#endif
            RuntimeDebugBindRow(runtime,button,workspace,UMI_DEBUG_SELECT_FRAME,index);
            gtk_list_box_append(
                GTK_LIST_BOX(runtime->debug_frames_list),
                button);
        }
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
/* Owned Framework targets and explicit child captures replace label-only variable rows. This preserves root values while preventing child IDs from overwriting roots or surviving an unrelated stop. The previous implementation remains for engineering review. */
#if 0
    if (runtime->debug_variables_list != NULL) {
        /* Visit each bounded item once so every record receives the same rule. */
        for (index = 0U; index < snapshot.visible_variable_count; ++index) {
            UmiDebugVariableSnapshot variable;
            char text[1700];

            (void)memset(&variable, 0, sizeof(variable));
            /* Apply this branch only when its contract condition is satisfied. */
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
#endif
    RuntimeDebugVariablesRender(runtime, workspace, &snapshot);
    RuntimeDebugScopesRender(runtime, workspace, &snapshot);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
/* Captured Framework watch intent and weak window actions replace label-only rows. Values remain explicit captures and editing never triggers execution. The previous implementation remains for engineering review. */
#if 0
    if (runtime->debug_watches_list != NULL) {
        /* Visit each bounded item once so every record receives the same rule. */
        for (index = 0U; index < snapshot.watch_count; ++index) {
            UmiDebugWatchSnapshot watch;
            char text[1800];

            (void)memset(&watch, 0, sizeof(watch));
            /* Apply this branch only when its contract condition is satisfied. */
            if (umi_debug_workspace_watch_at(
                    workspace,
                    index,
                    &watch) != UMI_STATUS_OK) {
                continue;
            }

            (void)snprintf(
                text,
                sizeof(text),
                "%.400s = %.900s  [%.300s]%s",
                watch.expression,
                watch.value,
                watch.type,
                watch.valid ? "" : "  invalid");
            gtk_list_box_append(
                GTK_LIST_BOX(runtime->debug_watches_list),
                runtime_label(text, 0.0F));
        }
    }

#endif
    /* Watch editing and explicit evaluation are rendered above. */
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
/* Interactive source properties use captured Framework edits and weak native actions. Retain the earlier labels to document their replacement. The previous implementation remains for engineering review. */
#if 0
    if (runtime->debug_breakpoints_list != NULL) {
        /* Visit each bounded item once so every record receives the same rule. */
        for (index = 0U; index < snapshot.breakpoint_count; ++index) {
            UmiDebugBreakpointSnapshot breakpoint;
            char text[1500];

            (void)memset(&breakpoint, 0, sizeof(breakpoint));
            /* Apply this branch only when its contract condition is satisfied. */
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
#endif
    /* The dedicated property renderer above replaces this label-only presentation. */
}

/*
 * Provide the runtime debug details notebook operation used by this module and its client
 * applications.
 */
static GtkWidget *runtime_debug_details_notebook(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *notebook;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return NULL;

    notebook = gtk_notebook_new();
    runtime->debug_threads_list = gtk_list_box_new();
    runtime->debug_frames_list = gtk_list_box_new();
    runtime->debug_variables_list = gtk_list_box_new();
    runtime->debug_watches_list = gtk_list_box_new();
    runtime->debug_breakpoints_list = gtk_list_box_new();

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
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
