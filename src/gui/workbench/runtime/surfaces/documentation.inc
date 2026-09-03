/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/documentation.inc
 *
 * PURPOSE:
 *   Present Framework browser navigation and readable local HTML previews in a
 *   dockable Studio panel.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#define UMI_STUDIO_DOCUMENTATION_MAX_SOURCE_BYTES (4U * 1024U * 1024U)

/* Return Studio's existing browser contract without creating another browser. */
static UmiBrowser *runtime_documentation_browser(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;
    UmiStudioWebPlatform *platform;
    if (runtime == NULL || runtime->owner == NULL) return NULL;
    services = umi_studio_ui_services(runtime->owner->ui);
    platform = services != NULL
        ? umi_studio_services_web_platform(services)
        : NULL;
    return platform != NULL
        ? umi_studio_web_platform_browser(platform)
        : NULL;
}

/* Replace the preview text and its short status message together. */
static void runtime_documentation_set_text(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *summary,
    const char *text)
{
    GtkTextBuffer *buffer;
    if (runtime == NULL) return;
    if (runtime->documentation_summary_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(runtime->documentation_summary_label),
            summary != NULL ? summary : "");
    }
    if (runtime->documentation_preview_view == NULL) return;
    buffer = gtk_text_view_get_buffer(
        GTK_TEXT_VIEW(runtime->documentation_preview_view));
    gtk_text_buffer_set_text(buffer, text != NULL ? text : "", -1);
}

/* Load one local document only after its size is known to be reasonable. */
static UmiStatus runtime_documentation_read_local(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *path)
{
    GStatBuf information;
    gchar *contents = NULL;
    gsize content_size = 0U;
    GError *error = NULL;
    UmiBrowserDocumentPreview preview;
    UmiStatus status;
    char summary[512];

    if (runtime == NULL || path == NULL || path[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    if (!g_file_test(path, G_FILE_TEST_IS_REGULAR) ||
        g_stat(path, &information) != 0) {
        runtime_documentation_set_text(
            runtime, "Document not found", path);
        return UMI_STATUS_NOT_FOUND;
    }
    /* Refuse unexpectedly large content before allocating a file-sized buffer. */
    if (information.st_size < 0 ||
        (uint64_t)information.st_size >
            (uint64_t)UMI_STUDIO_DOCUMENTATION_MAX_SOURCE_BYTES) {
        runtime_documentation_set_text(
            runtime,
            "Document is larger than the safe preview limit",
            "Open the file in the editor or use a dedicated browser renderer.");
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    if (!g_file_get_contents(path, &contents, &content_size, &error)) {
        runtime_documentation_set_text(
            runtime,
            "Document could not be read",
            error != NULL ? error->message : path);
        if (error != NULL) g_error_free(error);
        return UMI_STATUS_IO_ERROR;
    }

    status = umi_browser_document_preview_from_html(
        contents, (size_t)content_size, &preview);
    g_free(contents);
    if (status != UMI_STATUS_OK) {
        runtime_documentation_set_text(
            runtime, "Document preview failed", umi_status_text(status));
        return status;
    }
    (void)snprintf(
        summary,
        sizeof(summary),
        "%s%s · %zu source bytes · %zu readable bytes",
        preview.title[0] != '\0' ? preview.title : "Local documentation",
        preview.truncated ? " · preview shortened" : "",
        preview.source_bytes,
        preview.text_bytes);
    runtime_documentation_set_text(runtime, summary, preview.text);
    return UMI_STATUS_OK;
}

/* Display a location, optionally recording it in the shared browser history. */
static UmiStatus runtime_documentation_display_location(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *location,
    int record_navigation)
{
    UmiBrowser *browser = runtime_documentation_browser(runtime);
    gchar *local_path = NULL;
    gchar *browser_location = NULL;
    UmiStatus status = UMI_STATUS_OK;

    if (runtime == NULL || location == NULL || location[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    if (g_str_has_prefix(location, "file://")) {
        local_path = g_filename_from_uri(location, NULL, NULL);
    } else if (strstr(location, "://") == NULL) {
        local_path = g_canonicalize_filename(
            location,
            runtime->workspace_root[0] != '\0'
                ? runtime->workspace_root
                : NULL);
    }
    if (local_path != NULL) {
        browser_location = g_filename_to_uri(local_path, NULL, NULL);
    }

    if (record_navigation && browser != NULL && browser->navigate != NULL) {
        status = browser->navigate(
            browser->instance,
            browser_location != NULL ? browser_location : location);
    }
    if (runtime->documentation_location_entry != NULL) {
        gtk_editable_set_text(
            GTK_EDITABLE(runtime->documentation_location_entry),
            browser_location != NULL ? browser_location : location);
    }
    if (status == UMI_STATUS_OK && local_path != NULL) {
        status = runtime_documentation_read_local(runtime, local_path);
    } else if (status == UMI_STATUS_OK) {
        runtime_documentation_set_text(
            runtime,
            "Navigation recorded by the Framework browser",
            "This panel renders local HTML documentation as readable text. "
            "A native web renderer can implement the same browser contract "
            "for interactive remote pages without changing Studio commands.");
    }

    g_free(browser_location);
    g_free(local_path);
    return status;
}

/* Navigate to the location currently typed by the user. */
static void runtime_on_documentation_open(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *location;
    (void)button;
    if (runtime == NULL || runtime->documentation_location_entry == NULL) {
        return;
    }
    location = gtk_editable_get_text(
        GTK_EDITABLE(runtime->documentation_location_entry));
    (void)runtime_documentation_display_location(runtime, location, 1);
}

/* Find the first matching phrase in the current readable document preview. */
static void runtime_on_documentation_search(
    GtkSearchEntry *entry,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *query;
    GtkTextBuffer *buffer;
    GtkTextIter start;
    GtkTextIter match_start;
    GtkTextIter match_end;

    if (runtime == NULL || entry == NULL ||
        runtime->documentation_preview_view == NULL) {
        return;
    }
    query = gtk_editable_get_text(GTK_EDITABLE(entry));
    buffer = gtk_text_view_get_buffer(
        GTK_TEXT_VIEW(runtime->documentation_preview_view));
    gtk_text_buffer_get_start_iter(buffer, &start);
    /* An empty query clears the selection and leaves the document at its start. */
    if (query == NULL || query[0] == '\0') {
        gtk_text_buffer_place_cursor(buffer, &start);
        return;
    }
    /* Case-insensitive search is friendlier for prose and C identifiers. */
    if (gtk_text_iter_forward_search(
            &start, query, GTK_TEXT_SEARCH_CASE_INSENSITIVE,
            &match_start, &match_end, NULL)) {
        gtk_text_buffer_select_range(buffer, &match_start, &match_end);
        gtk_text_view_scroll_to_iter(
            GTK_TEXT_VIEW(runtime->documentation_preview_view),
            &match_start, 0.15, FALSE, 0.0, 0.0);
    }
}

/* Move through browser history and refresh the local readable preview. */
static void runtime_documentation_history_move(
    UmiStudioGtkRuntimeChrome *runtime,
    int forward)
{
    UmiBrowser *browser = runtime_documentation_browser(runtime);
    UmiStatus status;
    const char *current;
    if (browser == NULL || browser->instance == NULL) return;
    status = forward && browser->forward != NULL
        ? browser->forward(browser->instance)
        : (!forward && browser->back != NULL
            ? browser->back(browser->instance)
            : UMI_STATUS_NOT_FOUND);
    current = status == UMI_STATUS_OK && browser->current_url != NULL
        ? browser->current_url(browser->instance)
        : NULL;
    if (current != NULL) {
        (void)runtime_documentation_display_location(runtime, current, 0);
    }
}

static void runtime_on_documentation_back(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_documentation_history_move(
        (UmiStudioGtkRuntimeChrome *)user_data, 0);
}

static void runtime_on_documentation_forward(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_documentation_history_move(
        (UmiStudioGtkRuntimeChrome *)user_data, 1);
}

/* Ask the browser contract to reload, then rebuild the readable preview. */
static void runtime_on_documentation_reload(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiBrowser *browser = runtime_documentation_browser(runtime);
    const char *current;
    (void)button;
    if (browser == NULL || browser->reload == NULL ||
        browser->reload(browser->instance) != UMI_STATUS_OK) {
        return;
    }
    current = browser->current_url != NULL
        ? browser->current_url(browser->instance)
        : NULL;
    if (current != NULL) {
        (void)runtime_documentation_display_location(runtime, current, 0);
    }
}

/* Build the thin GTK adapter over Framework browser and preview services. */
static GtkWidget *runtime_build_surface_documentation(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Documentation");
    GtkWidget *navigation;
    GtkWidget *back;
    GtkWidget *forward;
    GtkWidget *reload;
    GtkWidget *open;

    if (panel == NULL || runtime == NULL) return panel;
    runtime->documentation_location_entry = gtk_entry_new();
    runtime->documentation_search_entry = gtk_search_entry_new();
    runtime->documentation_summary_label = runtime_label(
        "Enter an HTML file path or file URL.", 0.0F);
    runtime->documentation_preview_view = gtk_text_view_new();
    navigation = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    back = gtk_button_new_with_label("Back");
    forward = gtk_button_new_with_label("Forward");
    reload = gtk_button_new_with_label("Reload");
    open = gtk_button_new_with_label("Open");

    /* GTK normally returns a widget here, but keeping the guard makes this
     * presentation code safe if allocation fails during application startup. */
    if (navigation == NULL) return panel;

    if (runtime->documentation_location_entry != NULL) {
        gtk_entry_set_placeholder_text(
            GTK_ENTRY(runtime->documentation_location_entry),
            "docs/index.html or file:///path/to/page.html");
        gtk_widget_set_hexpand(runtime->documentation_location_entry, TRUE);
        gtk_box_append(
            GTK_BOX(navigation), runtime->documentation_location_entry);
        g_signal_connect(
            runtime->documentation_location_entry,
            "activate",
            G_CALLBACK(runtime_on_documentation_open), runtime);
    }
    if (back != NULL && forward != NULL && reload != NULL && open != NULL) {
        g_signal_connect(back, "clicked",
            G_CALLBACK(runtime_on_documentation_back), runtime);
        g_signal_connect(forward, "clicked",
            G_CALLBACK(runtime_on_documentation_forward), runtime);
        g_signal_connect(reload, "clicked",
            G_CALLBACK(runtime_on_documentation_reload), runtime);
        g_signal_connect(open, "clicked",
            G_CALLBACK(runtime_on_documentation_open), runtime);
        gtk_box_prepend(GTK_BOX(navigation), back);
        gtk_box_append(GTK_BOX(navigation), forward);
        gtk_box_append(GTK_BOX(navigation), reload);
        gtk_box_append(GTK_BOX(navigation), open);
    }
    gtk_box_append(GTK_BOX(panel), navigation);
    if (runtime->documentation_search_entry != NULL) {
        gtk_search_entry_set_placeholder_text(
            GTK_SEARCH_ENTRY(runtime->documentation_search_entry),
            "Find in documentation");
        g_signal_connect(
            runtime->documentation_search_entry,
            "search-changed",
            G_CALLBACK(runtime_on_documentation_search),
            runtime);
        gtk_box_append(
            GTK_BOX(panel), runtime->documentation_search_entry);
    }
    if (runtime->documentation_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->documentation_summary_label, "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel), runtime->documentation_summary_label);
    }
    if (runtime->documentation_preview_view != NULL) {
        gtk_text_view_set_editable(
            GTK_TEXT_VIEW(runtime->documentation_preview_view), FALSE);
        gtk_text_view_set_cursor_visible(
            GTK_TEXT_VIEW(runtime->documentation_preview_view), FALSE);
        gtk_text_view_set_wrap_mode(
            GTK_TEXT_VIEW(runtime->documentation_preview_view),
            GTK_WRAP_WORD_CHAR);
        gtk_widget_set_vexpand(runtime->documentation_preview_view, TRUE);
        gtk_box_append(
            GTK_BOX(panel), runtime_scroll(
                runtime->documentation_preview_view));
    }
    return panel;
}

#undef UMI_STUDIO_DOCUMENTATION_MAX_SOURCE_BYTES
