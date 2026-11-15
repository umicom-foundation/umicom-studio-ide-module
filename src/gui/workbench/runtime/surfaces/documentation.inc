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

#include "umicom/ui/gtk4/text_preview.h"

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
/* Framework now owns checked read-only publication; Studio re-resolves its owner after native observers The previous implementation is retained for engineering review. */
#if 0
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
#endif
static UmiStatus runtime_documentation_set_text(UmiStudioGtkRuntimeChrome *runtime,
    const char *summary, const char *text)
{
    if (runtime == NULL || runtime->owner == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    UmiStatus status = UMI_STATUS_CANCELLED;
    if (runtime != NULL) {
        if (runtime->documentation_summary_label == NULL || runtime->documentation_preview_view == NULL)
            status = UMI_STATUS_UNAVAILABLE;
        else status = UmiGtk4TextPreviewPublish(GTK_WINDOW(window),
            GTK_LABEL(runtime->documentation_summary_label),
            GTK_TEXT_VIEW(runtime->documentation_preview_view),
            summary != NULL ? summary : "", text != NULL ? text : "");
        if (RuntimeResolveOwner(request, &window) == NULL) status = UMI_STATUS_CANCELLED;
    }
    g_clear_object(&window); runtime_file_dialog_request_free(request);
    return status;
}

/* The former pathname check and unbounded second open are retained below for
 * review. Framework now enforces the byte budget on one open file handle and
 * rejects observable changes before Studio displays the resulting preview. */
#if 0
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

#endif

/* Return native publication cancellation while retaining the bounded reader and complete UTF-8 summary The previous implementation is retained for engineering review. */
#if 0
static UmiStatus runtime_documentation_read_local(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *path)
{
    UmiBrowserDocumentPreview preview;
    UmiStatus status;
    gchar *title;
    gchar *text;
    gchar *summary;
    if (runtime == NULL || path == NULL || path[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_browser_document_preview_read_file(
        path, UMI_STUDIO_DOCUMENTATION_MAX_SOURCE_BYTES, &preview);
    if (status != UMI_STATUS_OK) {
        runtime_documentation_set_text(runtime,
            status == UMI_STATUS_CAPACITY_EXCEEDED
                ? "Document is larger than the safe preview limit"
                : "Document could not be previewed",
            umi_status_text(status));
        return status;
    }
    /* The shared preview is byte-oriented and can end at a capacity boundary.
     * GTK requires UTF-8, so repair invalid or cut code points only at this
     * presentation boundary. The source file and Framework result stay intact. */
    title = g_utf8_make_valid(preview.title, -1);
    text = g_utf8_make_valid(preview.text, -1);
    /* Repaired UTF-8 can occupy more bytes than its source. Allocate the
     * small bounded summary instead of cutting a code point at a fixed limit. */
    summary = g_strdup_printf(
        "%s%s · %zu source bytes · %zu readable bytes",
        title[0] != '\0' ? title : "Local documentation",
        preview.truncated ? " · preview shortened" : "",
        preview.source_bytes, preview.text_bytes);
    runtime_documentation_set_text(runtime, summary, text);
    g_free(summary);
    g_free(text);
    g_free(title);
    return UMI_STATUS_OK;
}
#endif
static UmiStatus runtime_documentation_read_local(UmiStudioGtkRuntimeChrome *runtime, const char *path)
{
    if (runtime == NULL || path == NULL || path[0] == '\0') return UMI_STATUS_INVALID_ARGUMENT;
    UmiBrowserDocumentPreview preview;
    UmiStatus status = umi_browser_document_preview_read_file(
        path, UMI_STUDIO_DOCUMENTATION_MAX_SOURCE_BYTES, &preview);
    if (status != UMI_STATUS_OK) {
        UmiStatus published = runtime_documentation_set_text(runtime,
            status == UMI_STATUS_CAPACITY_EXCEEDED ? "Document is larger than the safe preview limit" :
            "Document could not be previewed", umi_status_text(status));
        return published == UMI_STATUS_OK ? status : published;
    }
    /* Keep byte-oriented extraction and its existing bounded file owner.
     * Repair UTF-8 only for native presentation, including the entire summary. */
    char *title = g_utf8_make_valid(preview.title, -1);
    char *text = g_utf8_make_valid(preview.text, -1);
    char *summary = g_strdup_printf("%s%s · %zu source bytes · %zu readable bytes",
        title[0] != '\0' ? title : "Local documentation",
        preview.truncated ? " · preview shortened" : "", preview.source_bytes, preview.text_bytes);
    status = runtime_documentation_set_text(runtime, summary, text);
    g_free(summary); g_free(text); g_free(title);
    return status;
}

/* Display a location, optionally recording it in the shared browser history. */
/* Relative documentation now requires an explicit project root; the former current-directory fallback is retained for review because application storage is not a source workspace. The previous implementation is retained for engineering review. */
#if 0
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
#endif

/* Copy borrowed entry/history text before any native callback can replace it. */
static UmiStatus RuntimeDocumentationCopyLocation(const char *location, char **out)
{
    *out = NULL;
    if (location == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    size_t bytes = 0U;
    while (bytes < UMI_PATH_CAPACITY && location[bytes] != '\0') ++bytes;
    if (bytes == 0U || bytes == UMI_PATH_CAPACITY || !g_utf8_validate(location, (gssize)bytes, NULL))
        return UMI_STATUS_INVALID_ARGUMENT;
    *out = g_try_malloc(bytes + 1U);
    if (*out == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    memcpy(*out, location, bytes + 1U);
    return UMI_STATUS_OK;
}
/* Copy navigation inputs and recheck owner identity after browser and address notifications; reject malformed file URLs explicitly The previous implementation is retained for engineering review. */
#if 0
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
        /* Relative documents need the user's source root. The process CWD
         * can be private settings storage and cannot supply project identity. */
        if (!g_path_is_absolute(location) && runtime->workspace_root[0] == '\0') {
            runtime_documentation_set_text(runtime, "Choose a document location",
                "Open a project for relative paths, enter a full document path, "
                "or choose a lesson library in Guided Learning.");
            return UMI_STATUS_INVALID_STATE;
        }
        local_path = g_canonicalize_filename(location,
            runtime->workspace_root[0] != '\0' ? runtime->workspace_root : NULL);
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
#endif
static UmiStatus runtime_documentation_display_location(
    UmiStudioGtkRuntimeChrome *runtime, const char *location, int record_navigation)
{
    if (runtime == NULL || (record_navigation != 0 && record_navigation != 1))
        return UMI_STATUS_INVALID_ARGUMENT;
    char *copied = NULL;
    UmiStatus status = RuntimeDocumentationCopyLocation(location, &copied);
    if (status != UMI_STATUS_OK) return status;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) { g_free(copied); return UMI_STATUS_OUT_OF_MEMORY; }
    GObject *window = NULL, *guard = NULL;
    GtkWidget *entry = NULL;
    char *local_path = NULL, *browser_location = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    if (g_object_get_data(window, "umicom-documentation-navigation-active") != NULL) {
        status = UMI_STATUS_BUSY; goto done;
    }
    guard = g_object_ref(window);
    g_object_set_data(guard, "umicom-documentation-navigation-active", GINT_TO_POINTER(1));
    if (g_str_has_prefix(copied, "file://")) {
        local_path = g_filename_from_uri(copied, NULL, NULL);
        if (local_path == NULL) { status = UMI_STATUS_INVALID_ARGUMENT; goto done; }
    } else if (strstr(copied, "://") == NULL) {
        if (!g_path_is_absolute(copied) && runtime->workspace_root[0] == '\0') {
            status = runtime_documentation_set_text(runtime, "Choose a document location",
                "Open a project for relative paths, enter a full document path, "
                "or choose a lesson library in Guided Learning.");
            if (status == UMI_STATUS_OK) status = UMI_STATUS_INVALID_STATE;
            goto done;
        }
        local_path = g_canonicalize_filename(copied,
            runtime->workspace_root[0] != '\0' ? runtime->workspace_root : NULL);
    }
    if (local_path != NULL) {
        browser_location = g_filename_to_uri(local_path, NULL, NULL);
        if (browser_location == NULL) { status = UMI_STATUS_INVALID_ARGUMENT; goto done; }
    }
    const char *displayed = browser_location != NULL ? browser_location : copied;
    UmiBrowser *browser = runtime_documentation_browser(runtime);
    if (record_navigation && browser != NULL && browser->navigate != NULL) {
        status = browser->navigate(browser->instance, displayed);
        runtime = RuntimeResolveOwner(request, &window);
        if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
        if (status != UMI_STATUS_OK) goto done;
    }
    if (runtime->documentation_location_entry == NULL) { status = UMI_STATUS_UNAVAILABLE; goto done; }
    entry = g_object_ref(runtime->documentation_location_entry);
    gtk_editable_set_text(GTK_EDITABLE(entry), displayed);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL || runtime->documentation_location_entry != entry ||
        gtk_widget_get_root(entry) != GTK_ROOT(window) ||
        strcmp(gtk_editable_get_text(GTK_EDITABLE(entry)), displayed) != 0) {
        status = UMI_STATUS_CANCELLED; goto done;
    }
    if (local_path != NULL) status = runtime_documentation_read_local(runtime, local_path);
    else status = runtime_documentation_set_text(runtime, "Navigation recorded by the Framework browser",
        "This panel renders local HTML documentation as readable text. "
        "A native web renderer can implement the same browser contract "
        "for interactive remote pages without changing Studio commands.");
done:
    if (guard != NULL) g_object_set_data(guard, "umicom-documentation-navigation-active", NULL);
    g_clear_object(&entry); g_clear_object(&guard); g_clear_object(&window);
    g_free(browser_location); g_free(local_path); g_free(copied);
    runtime_file_dialog_request_free(request);
    return status;
}

/* Navigate to the location currently typed by the user. */
/* Resolve the weak window binding on every activation so retained controls cannot reach a retired runtime. The previous implementation is retained for engineering review. */
#if 0
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
#endif
static void runtime_on_documentation_open(GtkWidget *source, gpointer data)
{
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &window);
    if (runtime != NULL && gtk_widget_get_root(source) == GTK_ROOT(window) &&
        runtime->documentation_location_entry != NULL) {
        const char *location = gtk_editable_get_text(GTK_EDITABLE(runtime->documentation_location_entry));
        (void)runtime_documentation_display_location(runtime, location, 1);
    }
    g_clear_object(&window);
}

/* Find the first matching phrase in the current readable document preview. */
/* Keep selection and iterator ownership in the shared preview service; search callbacks borrow no runtime after observers. The previous implementation is retained for engineering review. */
#if 0
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
#endif
static void runtime_on_documentation_search(GtkSearchEntry *entry, gpointer data)
{
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &window);
    if (runtime != NULL && runtime->documentation_search_entry == GTK_WIDGET(entry) &&
        gtk_widget_get_root(GTK_WIDGET(entry)) == GTK_ROOT(window) &&
        runtime->documentation_preview_view != NULL) {
        (void)UmiGtk4TextPreviewFind(GTK_WINDOW(window),
            GTK_TEXT_VIEW(runtime->documentation_preview_view),
            gtk_editable_get_text(GTK_EDITABLE(entry)));
    }
    g_clear_object(&window);
}

/* Move through browser history and refresh the local readable preview. */
/* Recheck the workbench after browser operations and copy borrowed history before address or preview observers run. The previous implementation is retained for engineering review. */
#if 0
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
#endif
static UmiStatus runtime_documentation_history_move(UmiStudioGtkRuntimeChrome *runtime, int action)
{
    if (runtime == NULL || action < 0 || action > 2) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GObject *window = NULL, *guard = NULL;
    char *location = NULL;
    UmiStatus status = UMI_STATUS_CANCELLED;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) goto done;
    if (g_object_get_data(window, "umicom-documentation-navigation-active") != NULL) {
        status = UMI_STATUS_BUSY; goto done;
    }
    guard = g_object_ref(window);
    g_object_set_data(guard, "umicom-documentation-navigation-active", GINT_TO_POINTER(1));
    UmiBrowser *browser = runtime_documentation_browser(runtime);
    status = UMI_STATUS_NOT_FOUND;
    if (browser == NULL || browser->instance == NULL) goto done;
    if (action == 0 && browser->back != NULL) status = browser->back(browser->instance);
    else if (action == 1 && browser->forward != NULL) status = browser->forward(browser->instance);
    else if (action == 2 && browser->reload != NULL) status = browser->reload(browser->instance);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    if (status != UMI_STATUS_OK) goto done;
    browser = runtime_documentation_browser(runtime);
    if (browser == NULL || browser->current_url == NULL) { status = UMI_STATUS_NOT_FOUND; goto done; }
    /* The browser owns this string. Copy before the next GTK notification. */
    const char *current = browser->current_url(browser->instance);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    status = RuntimeDocumentationCopyLocation(current, &location);
    if (status != UMI_STATUS_OK) goto done;
    g_object_set_data(guard, "umicom-documentation-navigation-active", NULL);
    g_clear_object(&guard);
    status = runtime_documentation_display_location(runtime, location, 0);
done:
    if (guard != NULL) g_object_set_data(guard, "umicom-documentation-navigation-active", NULL);
    g_clear_object(&guard); g_clear_object(&window); g_free(location);
    runtime_file_dialog_request_free(request);
    return status;
}

/* Retain only a weak owner identity in browser controls, including controls held after their panel closes. The previous implementation is retained for engineering review. */
#if 0
static void runtime_on_documentation_back(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_documentation_history_move(
        (UmiStudioGtkRuntimeChrome *)user_data, 0);
}
#endif
static void runtime_on_documentation_back(GtkButton *button, gpointer data)
{
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &window);
    if (runtime != NULL && gtk_widget_get_root(GTK_WIDGET(button)) == GTK_ROOT(window))
        (void)runtime_documentation_history_move(runtime, 0);
    g_clear_object(&window);
}

/* Retain only a weak owner identity in browser controls, including controls held after their panel closes. The previous implementation is retained for engineering review. */
#if 0
static void runtime_on_documentation_forward(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_documentation_history_move(
        (UmiStudioGtkRuntimeChrome *)user_data, 1);
}
#endif
static void runtime_on_documentation_forward(GtkButton *button, gpointer data)
{
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &window);
    if (runtime != NULL && gtk_widget_get_root(GTK_WIDGET(button)) == GTK_ROOT(window))
        (void)runtime_documentation_history_move(runtime, 1);
    g_clear_object(&window);
}

/* Ask the browser contract to reload, then rebuild the readable preview. */
/* Retain only a weak owner identity in browser controls, including controls held after their panel closes. The previous implementation is retained for engineering review. */
#if 0
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
#endif
static void runtime_on_documentation_reload(GtkButton *button, gpointer data)
{
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &window);
    if (runtime != NULL && gtk_widget_get_root(GTK_WIDGET(button)) == GTK_ROOT(window))
        (void)runtime_documentation_history_move(runtime, 2);
    g_clear_object(&window);
}

/* Build the thin GTK adapter over Framework browser and preview services. */
/* Give document navigation and its readable preview stable identities so the real first-open workflow can be checked independently of widget positions. The previous implementation is retained for engineering review. */
#if 0
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
#endif
/* Keep the existing Documentation layout while giving each navigation and search signal an owned weak binding. The previous implementation is retained for engineering review. */
#if 0
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
    /* Semantic identities support keyboard automation without inspecting labels. */
    (void)umi_gtk4_automation_tag_widget(runtime->documentation_location_entry, "studio.documentation.location");
    (void)umi_gtk4_automation_tag_widget(runtime->documentation_summary_label, "studio.documentation.summary");
    (void)umi_gtk4_automation_tag_widget(runtime->documentation_preview_view, "studio.documentation.preview");
    (void)umi_gtk4_automation_tag_widget(open, "studio.documentation.open");

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
#endif
/* Each signal closure owns its independent weak binding. Widgets retained by
 * automation or accessibility clients cannot prolong workbench authority. */
static void RuntimeDocumentationConnect(UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *widget, const char *signal, GCallback callback)
{
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) { gtk_widget_set_sensitive(widget, FALSE); return; }
    g_signal_connect_data(widget, signal, callback, request, RuntimeCommandButtonReleased, 0);
}
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
    /* Semantic identities support keyboard automation without inspecting labels. */
    (void)umi_gtk4_automation_tag_widget(runtime->documentation_location_entry, "studio.documentation.location");
    (void)umi_gtk4_automation_tag_widget(runtime->documentation_summary_label, "studio.documentation.summary");
    (void)umi_gtk4_automation_tag_widget(runtime->documentation_preview_view, "studio.documentation.preview");
    (void)umi_gtk4_automation_tag_widget(open, "studio.documentation.open");

    (void)umi_gtk4_automation_tag_widget(back, "studio.documentation.back");
    (void)umi_gtk4_automation_tag_widget(forward, "studio.documentation.forward");
    (void)umi_gtk4_automation_tag_widget(reload, "studio.documentation.reload");
    (void)umi_gtk4_automation_tag_widget(runtime->documentation_search_entry, "studio.documentation.search");

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
        RuntimeDocumentationConnect(runtime,
            runtime->documentation_location_entry,
            "activate",
            G_CALLBACK(runtime_on_documentation_open));
    }
    if (back != NULL && forward != NULL && reload != NULL && open != NULL) {
        RuntimeDocumentationConnect(runtime, back, "clicked",
            G_CALLBACK(runtime_on_documentation_back));
        RuntimeDocumentationConnect(runtime, forward, "clicked",
            G_CALLBACK(runtime_on_documentation_forward));
        RuntimeDocumentationConnect(runtime, reload, "clicked",
            G_CALLBACK(runtime_on_documentation_reload));
        RuntimeDocumentationConnect(runtime, open, "clicked",
            G_CALLBACK(runtime_on_documentation_open));
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
        RuntimeDocumentationConnect(runtime,
            runtime->documentation_search_entry,
            "search-changed",
            G_CALLBACK(runtime_on_documentation_search));
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
