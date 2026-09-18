/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/search.inc
 *
 * PURPOSE:
 *   Build the Framework-backed search presentation surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* The panel owns only presentation state. Its owner token prevents controls
 * retained after a layout/window change from addressing another runtime. */
typedef struct RuntimeFileSearchPanel {
    UmiStudioFileDialogRequest *owner;
    GtkWidget *panel;
    GtkWidget *query;
    GtkWidget *matchCase;
    GtkWidget *start;
    GtkWidget *cancel;
    GtkWidget *summary;
    GtkWidget *results;
    uint64_t renderedRequest;
    int renderedReady;
} RuntimeFileSearchPanel;

typedef struct RuntimeFileSearchRow {
    uint64_t requestId;
    size_t position;
} RuntimeFileSearchRow;

static void RuntimeFileSearchPanelFree(gpointer data)
{
    RuntimeFileSearchPanel *panel = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(panel->owner, &window);
    if (runtime != NULL && runtime->file_search_panel == panel->panel)
        runtime->file_search_panel = NULL;
    g_clear_object(&window);
    runtime_file_dialog_request_free(panel->owner);
    g_free(panel);
}

static UmiStudioGtkRuntimeChrome *RuntimeFileSearchOwner(RuntimeFileSearchPanel *panel,
    GObject **window)
{
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(panel->owner, window);
    return runtime != NULL && runtime->file_search_panel == panel->panel ? runtime : NULL;
}

static void RuntimeFileSearchOpen(GtkButton *button, gpointer data)
{
    RuntimeFileSearchPanel *panel = g_object_get_data(G_OBJECT(data), "umicom-file-search");
    RuntimeFileSearchRow *row = g_object_get_data(G_OBJECT(button), "umicom-search-row");
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeFileSearchOwner(panel, &window);
    if (runtime != NULL && row != NULL) {
        UmiStatus status = UmiStudioUiSearchOpen(runtime->owner->ui, row->requestId, row->position);
        if (status != UMI_STATUS_OK) runtime_show_notification(runtime,
            status == UMI_STATUS_NOT_FOUND
                ? "The saved match has moved or the file is unavailable. Any open draft is preserved; save and search again to update locations."
                : "This search result is no longer available. Search the current workspace again.");
        (void)runtime_refresh_services(runtime);
    }
    g_clear_object(&window);
}

static void RuntimeFileSearchRefresh(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->file_search_panel == NULL) return;
    RuntimeFileSearchPanel *panel = g_object_get_data(G_OBJECT(runtime->file_search_panel), "umicom-file-search");
    UmiFileSearchSnapshot state;
    UmiStatus status = UmiStudioUiSearchRead(runtime->owner->ui, &state);
    if (status != UMI_STATUS_OK) return;
    char *summary;
    if (state.stale) summary = g_strdup("The workspace or file index changed. Search again for current results.");
    else if (state.active) summary = g_strdup_printf("Searching saved files: %zu considered, %zu matches so far.", state.stats.files_considered, state.stats.matches);
    else if (state.requestId == 0U) summary = g_strdup("Enter text to search saved project files. Unsaved drafts are not searched or saved.");
    else if (state.status == UMI_STATUS_CANCELLED) summary = g_strdup("Search cancelled. No complete result set is shown.");
    else if (!state.ready) summary = g_strdup_printf("Search did not complete: %s. Check file access and refresh Explorer before retrying.", umi_status_text(state.status));
    else summary = g_strdup_printf("%zu matches in %zu searched files%s. Skipped: %zu binary, %zu oversized.",
        state.stats.matches, state.stats.files_searched,
        state.stats.truncated ? " (result limit reached; narrow the search)" : "",
        state.stats.binary_files_skipped, state.stats.oversized_files_skipped);
    gtk_label_set_text(GTK_LABEL(panel->summary), summary); g_free(summary);
    gtk_widget_set_sensitive(panel->start, !state.active);
    gtk_widget_set_sensitive(panel->cancel, state.active);
    if (panel->renderedRequest == state.requestId && panel->renderedReady == state.ready) return;
    runtime_clear_list_box(panel->results);
    panel->renderedRequest = state.requestId; panel->renderedReady = state.ready;
    if (!state.ready) return;
    for (size_t i = 0U; i < state.stats.matches; ++i) {
        UmiSearchMatch match;
        if (UmiStudioUiSearchMatchAt(runtime->owner->ui, state.requestId, i, &match) != UMI_STATUS_OK) break;
        char *path = g_utf8_make_valid(match.path, -1);
        char *preview = g_utf8_make_valid(match.preview, -1);
        char *labelText = g_strdup_printf("%s:%zu:%zu\n%s", path, match.line, match.column, preview);
        GtkWidget *result = gtk_button_new();
        GtkWidget *label = runtime_label(labelText, 0.0F);
        gtk_label_set_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
        gtk_button_set_child(GTK_BUTTON(result), label);
        gtk_widget_set_tooltip_text(result, path);
        RuntimeFileSearchRow *row = g_new(RuntimeFileSearchRow, 1);
        row->requestId = state.requestId; row->position = i;
        g_object_set_data_full(G_OBJECT(result), "umicom-search-row", row, g_free);
        g_signal_connect_object(result, "clicked", G_CALLBACK(RuntimeFileSearchOpen), panel->panel, 0);
        gtk_list_box_append(GTK_LIST_BOX(panel->results), result);
        g_free(path); g_free(preview); g_free(labelText);
    }
}

static void RuntimeFileSearchStart(gpointer widget, gpointer data)
{
    RuntimeFileSearchPanel *panel = g_object_get_data(G_OBJECT(data), "umicom-file-search");
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeFileSearchOwner(panel, &window);
    (void)widget;
    if (runtime != NULL) {
        UmiStatus status = UmiStudioUiSearchStart(runtime->owner->ui,
            gtk_editable_get_text(GTK_EDITABLE(panel->query)),
            gtk_check_button_get_active(GTK_CHECK_BUTTON(panel->matchCase)) ? 1 : 0);
        if (status != UMI_STATUS_OK) runtime_show_notification(runtime,
            status == UMI_STATUS_INVALID_ARGUMENT
                ? "Enter 1 to 1023 bytes of text on one line. This is literal text search, not a regular expression."
                : umi_status_text(status));
        RuntimeFileSearchRefresh(runtime);
    }
    g_clear_object(&window);
}

static void RuntimeFileSearchCancel(GtkButton *button, gpointer data)
{
    RuntimeFileSearchPanel *panel = g_object_get_data(G_OBJECT(data), "umicom-file-search");
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeFileSearchOwner(panel, &window);
    (void)button;
    if (runtime != NULL) {
        (void)UmiStudioUiSearchCancel(runtime->owner->ui);
        RuntimeFileSearchRefresh(runtime);
    }
    g_clear_object(&window);
}

static GtkWidget *runtime_build_surface_search(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Search");
    GtkWidget *button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || runtime == NULL) return panel;

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Search Everywhere",
            "Command/menu search is backed by UmiCommandRegistry and the "
            "Framework Application Shell. Use Ctrl+Shift+P or the button below."));

    button = runtime_make_local_button(
        runtime,
        "Open Command Search",
        "Search commands and menu contributions",
        0);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button != NULL) gtk_box_append(GTK_BOX(panel), button);

    RuntimeFileSearchPanel *search = g_new0(RuntimeFileSearchPanel, 1);
    search->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (search->owner == NULL) {
        g_free(search);
        gtk_box_append(GTK_BOX(panel), runtime_label("Search controls could not be created. Reopen this panel to retry.", 0.0F));
        return runtime_scroll(panel);
    }
    search->panel = panel; runtime->file_search_panel = panel;
    g_object_set_data_full(G_OBJECT(panel), "umicom-file-search", search, RuntimeFileSearchPanelFree);
    gtk_box_append(GTK_BOX(panel), runtime_card_new("Find in Files",
        "Search saved files in the open workspace. Results never replace unsaved editor text. Use Explorer Refresh to update the file list."));
    gtk_box_append(GTK_BOX(panel), runtime_label("Text to find", 0.0F));
    search->query = gtk_entry_new();
    gtk_widget_set_tooltip_text(search->query, "Literal text; maximum 1023 UTF-8 bytes. Press Enter to search.");
    g_object_set_data(G_OBJECT(search->query), "umicom-automation-id", "umicom-file-search-query");
    gtk_box_append(GTK_BOX(panel), search->query);
    search->matchCase = gtk_check_button_new_with_label("Match case");
    gtk_widget_set_tooltip_text(search->matchCase, "When unchecked, ASCII letters match without case. Unicode text is matched exactly.");
    gtk_box_append(GTK_BOX(panel), search->matchCase);
    GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    search->start = gtk_button_new_with_label("Find in Files");
    search->cancel = gtk_button_new_with_label("Cancel search");
    g_object_set_data(G_OBJECT(search->start), "umicom-automation-id", "umicom-file-search-start");
    g_object_set_data(G_OBJECT(search->cancel), "umicom-automation-id", "umicom-file-search-cancel");
    gtk_widget_set_tooltip_text(search->start, "Read saved project files on a background worker; do not save or run code.");
    gtk_widget_set_tooltip_text(search->cancel, "Cancel this search only; an active build or Explorer refresh continues.");
    gtk_box_append(GTK_BOX(actions), search->start); gtk_box_append(GTK_BOX(actions), search->cancel);
    gtk_box_append(GTK_BOX(panel), actions);
    search->summary = runtime_label("", 0.0F);
    gtk_label_set_wrap(GTK_LABEL(search->summary), TRUE);
    gtk_box_append(GTK_BOX(panel), search->summary);
    search->results = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(search->results), GTK_SELECTION_NONE);
    g_object_set_data(G_OBJECT(search->results), "umicom-automation-id", "umicom-file-search-results");
    gtk_box_append(GTK_BOX(panel), search->results);
    g_signal_connect_object(search->query, "activate", G_CALLBACK(RuntimeFileSearchStart), panel, 0);
    g_signal_connect_object(search->start, "clicked", G_CALLBACK(RuntimeFileSearchStart), panel, 0);
    g_signal_connect_object(search->cancel, "clicked", G_CALLBACK(RuntimeFileSearchCancel), panel, 0);
    RuntimeFileSearchRefresh(runtime);
    return runtime_scroll(panel);
}

