/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/search_pages.inc
 *
 * PURPOSE:
 *   Present captured saved-file search pages with checked owner and request identities.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/list_publication.h"
#include "umicom/ui/gtk4/window_lifecycle.h"

typedef struct RuntimeSearchPageCapture {
  UmiStudioFileDialogRequest *owner;
  GtkWidget *panel; /* Identity only; retained by the refresh caller. */
  UmiFileSearchPage *page;
  UmiFileSearchSnapshot state;
  UmiSearchPathFilter filter;
  size_t requested_offset;
} RuntimeSearchPageCapture;

static void RuntimeSearchPageFree(gpointer data) {
  RuntimeSearchPageCapture *capture = data;
  runtime_file_dialog_request_free(capture->owner);
  UmiFileSearchPageDestroy(capture->page);
  g_free(capture);
}
static UmiStatus RuntimeSearchPageCurrent(GtkListBox *list, void *data) {
  GObject *token = data;
  RuntimeSearchPageCapture *capture = g_object_get_data(token, "capture");
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(capture->owner, &window);
  UmiStatus status = UMI_STATUS_CANCELLED;
  if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window)) &&
      runtime->file_search_panel == capture->panel) {
    RuntimeFileSearchPanel *panel =
        g_object_get_data(G_OBJECT(capture->panel), "umicom-file-search");
    if (panel != NULL && panel->results == GTK_WIDGET(list) &&
        panel->offset == capture->requested_offset) {
      if (capture->page != NULL)
        status = UmiStudioUiSearchPageValidate(runtime->owner->ui, capture->page);
      else {
        UmiFileSearchSnapshot now;
        status = UmiStudioUiSearchRead(runtime->owner->ui, &now);
        if (status == UMI_STATUS_OK &&
            (now.requestId != capture->state.requestId ||
             now.indexRevision != capture->state.indexRevision ||
             now.active != capture->state.active || now.ready != capture->state.ready ||
             now.stale != capture->state.stale || now.status != capture->state.status))
          status = UMI_STATUS_INVALID_STATE;
      }
    }
  }
  g_clear_object(&window);
  return status;
}
static void RuntimeSearchPageOpen(GtkButton *button, gpointer data) {
  GObject *token = g_object_ref(data);
  RuntimeSearchPageCapture *capture = g_object_get_data(token, "capture");
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(capture->owner, &window);
  if (runtime != NULL && runtime->file_search_panel == capture->panel) {
    RuntimeFileSearchPanel *panel =
        g_object_get_data(G_OBJECT(capture->panel), "umicom-file-search");
    GtkWidget *row = gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_LIST_BOX_ROW);
    RuntimeFileSearchRow *identity = g_object_get_data(G_OBJECT(button), "umicom-search-row");
    if (panel != NULL && !panel->publishing && panel->published == token && identity != NULL &&
        row != NULL && gtk_widget_get_parent(row) == panel->results &&
        (gpointer)gtk_widget_get_root(GTK_WIDGET(button)) == (gpointer)window &&
        RuntimeSearchPageCurrent(GTK_LIST_BOX(panel->results), token) == UMI_STATUS_OK) {
      /* Navigation can replace widgets or close the host. Its request
       * identity has already been copied; only reacquired owners receive feedback. */
      UmiStatus status =
          UmiStudioUiSearchOpen(runtime->owner->ui, identity->requestId, identity->position);
      g_clear_object(&window);
      runtime = runtime_file_dialog_owner(capture->owner, &window);
      if (runtime != NULL && status == UMI_STATUS_OK)
        status = RuntimePresentActiveSource(runtime);
      g_clear_object(&window);
      runtime = runtime_file_dialog_owner(capture->owner, &window);
      if (runtime != NULL && status != UMI_STATUS_OK && status != UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime, "The saved match could not be opened at its captured "
                                           "location. Keep any open draft and search again.");
    }
  }
  g_clear_object(&window);
  g_object_unref(token);
}
static void RuntimeFileSearchRefresh(UmiStudioGtkRuntimeChrome *runtime) {
  if (runtime == NULL || runtime->file_search_panel == NULL)
    return;
  GtkWidget *root = g_object_ref(runtime->file_search_panel);
  RuntimeFileSearchPanel *panel = g_object_get_data(G_OBJECT(root), "umicom-file-search");
  if (panel == NULL || panel->publishing) {
    g_object_unref(root);
    return;
  }
  panel->publishing = 1;
  GObject *token = g_object_new(G_TYPE_OBJECT, NULL);
  RuntimeSearchPageCapture *capture = g_new0(RuntimeSearchPageCapture, 1);
  capture->owner = runtime_file_dialog_request_new(runtime, 0U);
  capture->panel = root;
  g_object_set_data_full(token, "capture", capture, RuntimeSearchPageFree);
  GtkWidget *rows[UMI_FILE_SEARCH_PAGE_LIMIT] = {0};
  size_t row_count = 0U;
  UmiStatus status = capture->owner == NULL
                         ? UMI_STATUS_OUT_OF_MEMORY
                         : UmiStudioUiSearchRead(runtime->owner->ui, &capture->state);
  if (status != UMI_STATUS_OK)
    goto finished;
  if (panel->renderedRequest != capture->state.requestId)
    panel->offset = 0U;
  capture->requested_offset = panel->offset;
  UmiFileSearchPageSummary page = {0};
  if (capture->state.ready && !capture->state.stale && !capture->state.active) {
    status = UmiStudioUiSearchPageCreate(runtime->owner->ui, panel->offset, 0U, &capture->page);
    if (status != UMI_STATUS_OK)
      goto finished;
    (void)UmiFileSearchPageRead(capture->page, &page);
    panel->offset = page.offset;
    capture->requested_offset = page.offset;
    capture->filter = page.filter;
  } else if (capture->state.requestId != 0U)
    (void)UmiStudioUiSearchFilterRead(runtime->owner->ui, capture->state.requestId,
                                      &capture->filter);
  /* Keep the same row identities on a periodic refresh of an unchanged page. */
  if (panel->published != NULL && capture->page != NULL &&
      RuntimeSearchPageCurrent(GTK_LIST_BOX(panel->results), panel->published) == UMI_STATUS_OK)
    goto finished;

  char *summary;
  if (capture->state.stale)
    summary = g_strdup("The workspace or file index changed. Search again for current results.");
  else if (capture->state.active)
    summary = g_strdup_printf("Searching saved files: %zu considered, %zu matches so far.",
                              capture->state.stats.files_considered, capture->state.stats.matches);
  else if (capture->state.requestId == 0U)
    summary = g_strdup(
        "Enter text to search saved project files. Unsaved drafts are not searched or saved.");
  else if (capture->state.status == UMI_STATUS_CANCELLED)
    summary = g_strdup("Search cancelled. No complete result set is shown.");
  else if (!capture->state.ready)
    summary =
        g_strdup_printf("Search did not complete: %s.", umi_status_text(capture->state.status));
  else
    summary = g_strdup_printf(
        "%zu matches in %zu searched files%s. Skipped: %zu binary or unsupported text, %zu "
        "oversized.",
        capture->state.stats.matches, capture->state.stats.files_searched,
        capture->state.stats.truncated ? " (result limit reached; narrow the search)" : "",
        capture->state.stats.binary_files_skipped, capture->state.stats.oversized_files_skipped);
  char *scope = g_strdup_printf(
      "Captured query: %s\nInclude: %s; exclude: %s", capture->state.query,
      capture->filter.include_patterns[0] ? capture->filter.include_patterns : "all indexed files",
      capture->filter.exclude_patterns[0] ? capture->filter.exclude_patterns : "none");
  char *range = page.count == 0U
                    ? g_strdup("No result page")
                    : g_strdup_printf("Results %zu–%zu of %zu · Page %zu of %zu", page.offset + 1U,
                                      page.offset + page.count, page.search.stats.matches,
                                      page.page_index, page.page_count);
  /* Every setter may notify application code. The snapshot owns all text and
   * the panel owns widget references; no runtime pointer crosses a setter. */
#define SEARCH_CURRENT()                                                                           \
  (RuntimeSearchPageCurrent(GTK_LIST_BOX(panel->results), token) == UMI_STATUS_OK)
  if (SEARCH_CURRENT())
    gtk_label_set_text(GTK_LABEL(panel->summary), summary);
  if (SEARCH_CURRENT())
    gtk_label_set_text(GTK_LABEL(panel->capturedScope), scope);
  if (SEARCH_CURRENT())
    gtk_label_set_text(GTK_LABEL(panel->range), range);
  if (SEARCH_CURRENT())
    gtk_widget_set_sensitive(panel->start, !capture->state.active);
  if (SEARCH_CURRENT())
    gtk_widget_set_sensitive(panel->cancel, capture->state.active);
  if (SEARCH_CURRENT())
    gtk_widget_set_sensitive(panel->first, page.has_previous);
  if (SEARCH_CURRENT())
    gtk_widget_set_sensitive(panel->previous, page.has_previous);
  if (SEARCH_CURRENT())
    gtk_widget_set_sensitive(panel->next, page.has_next);
  if (SEARCH_CURRENT())
    gtk_widget_set_sensitive(panel->last, page.has_next);
  g_free(summary);
  g_free(scope);
  g_free(range);
  if (!SEARCH_CURRENT()) {
    status = UMI_STATUS_CANCELLED;
    goto finished;
  }
  for (size_t i = 0U; i < page.count; ++i) {
    UmiSearchMatch match;
    status = UmiFileSearchPageMatchAt(capture->page, i, &match);
    if (status != UMI_STATUS_OK)
      goto finished;
    char *path = g_utf8_make_valid(match.path, -1);
    char *preview = g_utf8_make_valid(match.preview, -1);
    char *text = g_strdup_printf("%s:%zu:%zu\n%s", path, match.line, match.column, preview);
    GtkWidget *button = gtk_button_new();
    g_object_ref_sink(button);
    rows[row_count++] = button;
    GtkWidget *label = runtime_label(text, 0.0F);
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
    gtk_button_set_child(GTK_BUTTON(button), label);
    gtk_widget_set_tooltip_text(button, path);
    RuntimeFileSearchRow *identity = g_new(RuntimeFileSearchRow, 1);
    identity->requestId = page.search.requestId;
    identity->position = page.offset + i;
    g_object_set_data_full(G_OBJECT(button), "umicom-search-row", identity, g_free);
    g_object_set_data_full(G_OBJECT(button), "studio-search-page", g_object_ref(token),
                           g_object_unref);
    g_signal_connect(button, "clicked", G_CALLBACK(RuntimeSearchPageOpen), token);
    g_free(path);
    g_free(preview);
    g_free(text);
  }
  /* Invalidate actions before rows are removed. A retained row from an old
   * page never becomes permission to navigate a newer search. */
  g_clear_object(&panel->published);
  status = UmiGtk4ListBoxPublish(GTK_LIST_BOX(panel->results), rows, row_count,
                                 RuntimeSearchPageCurrent, token);
  if (status == UMI_STATUS_OK && SEARCH_CURRENT()) {
    panel->published = g_object_ref(token);
    panel->renderedRequest = capture->state.requestId;
    panel->renderedReady = capture->state.ready;
  } else if (status == UMI_STATUS_OK)
    status = UMI_STATUS_CANCELLED;
#undef SEARCH_CURRENT
finished:
  if (status != UMI_STATUS_OK) {
    g_clear_object(&panel->published);
    panel->renderedReady = 0;
  }
  for (size_t i = 0U; i < row_count; ++i)
    g_object_unref(rows[i]);
  g_object_unref(token);
  panel->publishing = 0;
  g_object_unref(root);
}
static void RuntimeSearchPageMove(GtkButton *button, gpointer data) {
  GtkWidget *root = g_object_ref(data);
  RuntimeFileSearchPanel *panel = g_object_get_data(G_OBJECT(root), "umicom-file-search");
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime =
      panel != NULL ? RuntimeFileSearchOwner(panel, &window) : NULL;
  if (runtime != NULL && !panel->publishing && panel->published != NULL &&
      (gpointer)gtk_widget_get_root(GTK_WIDGET(button)) == (gpointer)window &&
      RuntimeSearchPageCurrent(GTK_LIST_BOX(panel->results), panel->published) == UMI_STATUS_OK) {
    RuntimeSearchPageCapture *capture = g_object_get_data(panel->published, "capture");
    UmiFileSearchPageSummary page;
    if (UmiFileSearchPageRead(capture->page, &page) == UMI_STATUS_OK) {
      int action = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "studio-search-move"));
      if (action == 0)
        panel->offset = 0U;
      else if (action == 1)
        panel->offset = page.previous_offset;
      else if (action == 2)
        panel->offset = page.next_offset;
      else if (action == 3)
        panel->offset = page.page_count == 0U ? 0U : (page.page_count - 1U) * page.limit;
      RuntimeFileSearchRefresh(runtime);
    }
  }
  g_clear_object(&window);
  g_object_unref(root);
}
static void RuntimeFileSearchStart(gpointer widget, gpointer data) {
  (void)widget;
  GtkWidget *root = g_object_ref(data);
  RuntimeFileSearchPanel *panel = g_object_get_data(G_OBJECT(root), "umicom-file-search");
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime =
      panel != NULL ? RuntimeFileSearchOwner(panel, &window) : NULL;
  if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window)) && !panel->publishing) {
    panel->publishing = 1;
    UmiSearchPathFilter filter;
    UmiStatus status = UmiSearchPathFilterInit(
        gtk_editable_get_text(GTK_EDITABLE(panel->includePatterns)),
        gtk_editable_get_text(GTK_EDITABLE(panel->excludePatterns)), &filter);
    char *query = g_strdup(gtk_editable_get_text(GTK_EDITABLE(panel->query)));
    int match_case = gtk_check_button_get_active(GTK_CHECK_BUTTON(panel->matchCase)) ? 1 : 0;
    const char *message = NULL;
    if (status != UMI_STATUS_OK)
      message = "Use up to 16 semicolon-separated relative patterns per field, such as *.c;*.h or "
                "src/*.c. Empty items, brackets and braces are unsupported.";
    else {
      status = UmiStudioUiSearchStartFiltered(runtime->owner->ui, query, match_case, &filter);
      if (status != UMI_STATUS_OK)
        message = status == UMI_STATUS_INVALID_ARGUMENT
                      ? "Enter 1 to 1023 bytes of literal text on one line."
                      : umi_status_text(status);
    }
    g_free(query);
    g_clear_object(&window);
    runtime = RuntimeFileSearchOwner(panel, &window);
    if (runtime != NULL && message != NULL)
      runtime_show_notification(runtime, message);
    panel->publishing = 0;
    g_clear_object(&window);
    runtime = RuntimeFileSearchOwner(panel, &window);
    if (runtime != NULL)
      RuntimeFileSearchRefresh(runtime);
  }
  g_clear_object(&window);
  g_object_unref(root);
}
static void RuntimeFileSearchCancel(GtkButton *button, gpointer data) {
  (void)button;
  GtkWidget *root = g_object_ref(data);
  RuntimeFileSearchPanel *panel = g_object_get_data(G_OBJECT(root), "umicom-file-search");
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime =
      panel != NULL ? RuntimeFileSearchOwner(panel, &window) : NULL;
  if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window)) && !panel->publishing) {
    panel->publishing = 1;
    (void)UmiStudioUiSearchCancel(runtime->owner->ui);
    panel->publishing = 0;
    g_clear_object(&window);
    runtime = RuntimeFileSearchOwner(panel, &window);
    if (runtime != NULL)
      RuntimeFileSearchRefresh(runtime);
  }
  g_clear_object(&window);
  g_object_unref(root);
}
