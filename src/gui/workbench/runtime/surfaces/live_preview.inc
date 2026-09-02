/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/live_preview.inc
 *
 * PURPOSE:
 *   Present Framework live-preview results for the active editor document.
 *   This file owns GTK widgets only; matching, rendering and safety policy
 *   remain inside the reusable Framework source-preview service.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Replace the visible result and explanatory status as one UI update. */
static void runtime_live_preview_set_content(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *summary,
    const char *content,
    int monospace)
{
    GtkTextBuffer *buffer;

    /* A disappearing panel during shutdown needs no further GTK update. */
    if (runtime == NULL) return;
    /* The summary is optional while the panel is being constructed. */
    if (runtime->live_preview_summary_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(runtime->live_preview_summary_label),
            summary != NULL ? summary : "");
    }
    /* The text view may not exist if widget allocation failed partway through. */
    if (runtime->live_preview_text_view == NULL) return;
    buffer = gtk_text_view_get_buffer(
        GTK_TEXT_VIEW(runtime->live_preview_text_view));
    gtk_text_buffer_set_text(
        buffer,
        content != NULL ? content : "",
        -1);
    gtk_text_view_set_monospace(
        GTK_TEXT_VIEW(runtime->live_preview_text_view),
        monospace != 0);
}

/*
 * Resolve the active editor view and ask Framework for its best permitted
 * preview. The document-model revision avoids repeating work on timer ticks
 * where neither the text nor the selected editor changed.
 */
static UmiStatus runtime_live_preview_refresh(
    UmiStudioGtkRuntimeChrome *runtime,
    int force)
{
    UmiDocumentWorkingCopySnapshot active;
    UmiUiDocumentViewModel *documents;
    UmiUiDocumentViewSnapshot view;
    UmiBrowserSourcePreviewRequest request;
    UmiBrowserSourcePreviewResult *result;
    uint64_t model_revision;
    const char *preview_path;
    char summary[1024];
    UmiStatus status;

    /* Rendering requires both the reusable service and an attached GTK view. */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->source_preview_service == NULL ||
        runtime->live_preview_text_view == NULL) {
        return UMI_STATUS_UNAVAILABLE;
    }

    /* A hidden panel does not consume rendering work during ordinary refresh
     * ticks. Opening it uses a forced refresh, so its content is immediately
     * current whether it is docked or floating. */
    if (!force &&
        !runtime->surface_visible[UMI_STUDIO_SURFACE_LIVE_PREVIEW]) {
        return UMI_STATUS_OK;
    }

    documents = umi_ui_workbench_documents(
        umi_studio_ui_workbench(runtime->owner->ui));
    /* Studio can attach before the document model is ready during recovery. */
    if (documents == NULL) return UMI_STATUS_UNAVAILABLE;
    model_revision = umi_ui_document_view_model_revision(documents);

    /* Once the empty-state guidance has been shown, keep it until the document
     * model changes instead of rebuilding the same text every timer tick. */
    if (!force && model_revision == runtime->live_preview_model_revision &&
        runtime->live_preview_view_id[0] == '\0') {
        return UMI_STATUS_OK;
    }

    (void)memset(&active, 0, sizeof(active));
    status = umi_document_coordinator_active_snapshot(
        runtime_documents(runtime), &active);
    /* With no active tab, show guidance instead of treating it as an error. */
    if (status != UMI_STATUS_OK) {
        runtime->live_preview_model_revision = model_revision;
        runtime->live_preview_view_id[0] = '\0';
        runtime_live_preview_set_content(
            runtime,
            "Open a document to begin previewing.",
            "Live Preview follows the active editor and updates after the "
            "document changes.",
            0);
        return UMI_STATUS_OK;
    }

    /* A document-model revision changes for text edits, activation and tab
     * state. Comparing the view ID as well prevents a newly selected document
     * from reusing an earlier document's result. */
    if (!force && model_revision == runtime->live_preview_model_revision &&
        strcmp(active.view_id, runtime->live_preview_view_id) == 0) {
        return UMI_STATUS_OK;
    }

    status = umi_ui_document_view_model_find(
        documents, active.view_id, &view);
    /* A coordinator snapshot must resolve to the same document-view record. */
    if (status != UMI_STATUS_OK) return status;
    /* Never call strlen unless the bounded presentation copy is terminated. */
    if (memchr(view.source_text, '\0', sizeof(view.source_text)) == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }

    umi_browser_source_preview_request_init(&request);
    preview_path = active.has_path && active.path[0] != '\0'
        ? active.path
        : view.title;
    request.path = preview_path;
    request.language_id = view.language_id;
    request.source = view.source_text;
    request.source_length = strlen(view.source_text);
    request.source_revision = model_revision != 0U ? model_revision : 1U;
    /* Studio's built-in preview never executes source. Future sandbox
     * providers require an explicit user action and a trusted workspace. */
    request.workspace_trusted = 0;
    request.allow_source_execution = 0;

    result = (UmiBrowserSourcePreviewResult *)g_try_malloc0(sizeof(*result));
    /* Keep the large bounded result off the call stack used by GTK callbacks. */
    if (result == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_browser_source_preview_service_render(
        runtime->source_preview_service, &request, result);
    /* Only a validated Framework result is copied into visible GTK widgets. */
    if (status == UMI_STATUS_OK) {
        int written = snprintf(
            summary,
            sizeof(summary),
            "%s · %s · revision %llu · %zu source bytes%s\n%s",
            result->title,
            umi_browser_source_preview_kind_text(result->kind),
            (unsigned long long)result->source_revision,
            result->source_bytes,
            result->truncated ? " · output shortened" : "",
            result->summary);
        /* A formatting failure is surfaced rather than showing stale metadata. */
        if (written < 0) {
            status = UMI_STATUS_INVALID_STATE;
        } else {
            runtime_live_preview_set_content(
                runtime,
                summary,
                result->content,
                result->kind == UMI_BROWSER_SOURCE_PREVIEW_TEXT);
            runtime->live_preview_model_revision = model_revision;
            (void)g_strlcpy(
                runtime->live_preview_view_id,
                active.view_id,
                sizeof(runtime->live_preview_view_id));
        }
    }
    g_free(result);

    /* Preserve the editor and replace only the preview with useful guidance. */
    if (status != UMI_STATUS_OK) {
        (void)snprintf(
            summary,
            sizeof(summary),
            "Preview unavailable for %s: %s",
            view.title,
            umi_status_text(status));
        runtime_live_preview_set_content(
            runtime,
            summary,
            "A compatible preview provider is not currently available. The "
            "source document remains open and unchanged.",
            0);
    }
    return status;
}

/* Refresh immediately when the user requests it instead of waiting for a timer tick. */
static void runtime_on_live_preview_refresh(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStatus status;
    (void)button;

    status = runtime_live_preview_refresh(runtime, 1);
    /* Missing providers are explained inside the panel; other failures also
     * receive the normal Studio notification treatment. */
    if (status != UMI_STATUS_OK && status != UMI_STATUS_NOT_FOUND) {
        runtime_show_notification(runtime, umi_status_text(status));
    }
}

/* Build a movable panel that remains useful in docked and detached layouts. */
static GtkWidget *runtime_build_surface_live_preview(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Live Preview");
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *refresh = gtk_button_new_with_label("Refresh");
    GtkWidget *automatic = runtime_label(
        "Follows the active editor without executing source.", 0.0F);
    GtkWidget *scroll;

    /* Return the most complete panel available if widget allocation fails. */
    if (panel == NULL || toolbar == NULL || refresh == NULL ||
        runtime == NULL) {
        return panel;
    }

    runtime->live_preview_summary_label = runtime_label(
        "Open a document to begin previewing.", 0.0F);
    runtime->live_preview_text_view = gtk_text_view_new();
    /* Both widgets are required because refresh updates them as one result. */
    if (runtime->live_preview_summary_label == NULL ||
        runtime->live_preview_text_view == NULL) {
        return panel;
    }
    scroll = runtime_scroll(runtime->live_preview_text_view);
    /* A missing scroll container leaves the partially built panel harmless. */
    if (scroll == NULL) return panel;

    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(runtime->live_preview_text_view), FALSE);
    gtk_text_view_set_cursor_visible(
        GTK_TEXT_VIEW(runtime->live_preview_text_view), FALSE);
    gtk_text_view_set_wrap_mode(
        GTK_TEXT_VIEW(runtime->live_preview_text_view), GTK_WRAP_WORD_CHAR);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_widget_set_tooltip_text(
        refresh,
        "Rebuild the preview from the current editor text");
    g_signal_connect(
        refresh,
        "clicked",
        G_CALLBACK(runtime_on_live_preview_refresh),
        runtime);

    gtk_box_append(GTK_BOX(toolbar), refresh);
    /* The explanatory label is helpful but not required for operation. */
    if (automatic != NULL) gtk_box_append(GTK_BOX(toolbar), automatic);
    gtk_box_append(GTK_BOX(panel), toolbar);
    gtk_box_append(
        GTK_BOX(panel), runtime->live_preview_summary_label);
    gtk_box_append(GTK_BOX(panel), scroll);
    (void)runtime_live_preview_refresh(runtime, 1);
    return panel;
}
