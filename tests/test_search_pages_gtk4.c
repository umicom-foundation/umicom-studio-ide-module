/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_search_pages_gtk4.c
 *
 * PURPOSE:
 *   Exercise native page navigation and owner retirement during search publication.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "search_pages_fixture.h"
static void Observed(GObject *object, GParamSpec *property, gpointer data) {
  (void)object;
  (void)property;
  SearchPagesFixture *f = data;
  if (f->callbacks++ != 0U)
    return;
  if (strcmp(f->action, "refresh-reentry") == 0)
    /* This check observes native controls. Preserve the projection-only
     * refresh for review; explicit workspace synchronization publishes the
     * search pane and command availability without waiting for a timer. */
#if 0
    (void)umi_studio_gtk_workbench_refresh(f->workbench);
#endif
    (void)umi_studio_gtk_workbench_workspace_synchronise(f->workbench);
  else if (strcmp(f->action, "start-reentry") == 0)
    SearchClick(f, "start");
  else if (strcmp(f->action, "workspace-during-publication") == 0)
    CHECK(umi_studio_workspace_open(umi_studio_bootstrap_services(f->bootstrap), f->workspace, 0,
                                    0) == UMI_STATUS_OK);
  else {
    UmiStudioGtkWorkbench *retired = f->workbench;
    f->workbench = NULL;
    f->root = NULL;
    umi_studio_gtk_workbench_destroy(retired);
  }
}
static void RowsChanged(GObject *object, GParamSpec *property, gpointer data) {
  Observed(object, property, data);
}
int main(int argc, char **argv) {
  CHECK(argc == 2);
  const char *known[] = {"normal",
                         "next",
                         "previous",
                         "last",
                         "first",
                         "unchanged",
                         "old-row",
                         "new-request",
                         "stale-index",
                         "filter-query",
                         "empty",
                         "retired",
                         "summary-close",
                         "scope-close",
                         "range-close",
                         "rows-close",
                         "refresh-reentry",
                         "start-reentry",
                         "workspace-during-publication",
                         "invalid-retired"};
  int recognised = 0;
  for (size_t i = 0U; i < sizeof known / sizeof known[0]; ++i)
    if (strcmp(argv[1], known[i]) == 0)
      recognised = 1;
  CHECK(recognised);
  (void)g_setenv("GTK_A11Y", "test", TRUE);
  if (!gtk_init_check())
    return 77;
  SearchPagesFixture f;
  SearchPagesCreate(&f);
  f.action = argv[1];
  CHECK(SearchRowCount(&f) == 64U);
  CHECK(strstr(gtk_label_get_text(GTK_LABEL(SearchControl(&f, "range"))), "Page 1 of 3") != NULL);
  if (strcmp(f.action, "next") == 0 || strcmp(f.action, "previous") == 0 ||
      strcmp(f.action, "first") == 0) {
    SearchClick(&f, "next");
    CHECK(SearchRowCount(&f) == 64U);
    GtkWidget *label = gtk_button_get_child(GTK_BUTTON(SearchFirstButton(&f)));
    CHECK(strstr(gtk_label_get_text(GTK_LABEL(label)), ":65:1") != NULL);
    if (strcmp(f.action, "next") != 0) {
      SearchClick(&f, strcmp(f.action, "first") == 0 ? "first" : "previous");
      CHECK(strstr(gtk_label_get_text(GTK_LABEL(SearchControl(&f, "range"))), "Page 1 of 3") !=
            NULL);
    }
  } else if (strcmp(f.action, "last") == 0) {
    SearchClick(&f, "last");
    CHECK(SearchRowCount(&f) == 22U);
    CHECK(!gtk_widget_get_sensitive(SearchControl(&f, "next")));
  } else if (strcmp(f.action, "unchanged") == 0) {
    GtkWidget *old = g_object_ref(SearchFirstButton(&f));
    /* This check observes native controls. Preserve the projection-only
     * refresh for review; explicit workspace synchronization publishes the
     * search pane and command availability without waiting for a timer. */
#if 0
    CHECK(umi_studio_gtk_workbench_refresh(f.workbench) == UMI_STATUS_OK);
#endif
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(f.workbench) == UMI_STATUS_OK);
    CHECK(old == SearchFirstButton(&f));
    g_object_unref(old);
  } else if (strcmp(f.action, "old-row") == 0) {
    GtkWidget *old = g_object_ref(SearchFirstButton(&f));
    UmiStudioUiSnapshot before, after;
    CHECK(umi_studio_ui_snapshot(f.ui, &before) == UMI_STATUS_OK);
    SearchClick(&f, "next");
    g_signal_emit_by_name(old, "clicked");
    CHECK(umi_studio_ui_snapshot(f.ui, &after) == UMI_STATUS_OK &&
          before.documents == after.documents);
    g_object_unref(old);
  } else if (strcmp(f.action, "new-request") == 0 || strcmp(f.action, "empty") == 0) {
    SearchClick(&f, "last");
    gtk_editable_set_text(GTK_EDITABLE(SearchControl(&f, "query")),
                          strcmp(f.action, "empty") == 0 ? "AbsentMarker" : "Page");
    SearchClick(&f, "start");
    SearchWait(&f);
    /* This check observes native controls. Preserve the projection-only
     * refresh for review; explicit workspace synchronization publishes the
     * search pane and command availability without waiting for a timer. */
#if 0
    CHECK(umi_studio_gtk_workbench_refresh(f.workbench) == UMI_STATUS_OK);
#endif
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(f.workbench) == UMI_STATUS_OK);
    CHECK(SearchRowCount(&f) == (strcmp(f.action, "empty") == 0 ? 0U : 64U));
    if (strcmp(f.action, "empty") != 0)
      CHECK(strstr(gtk_label_get_text(GTK_LABEL(SearchControl(&f, "range"))), "Page 1 of 3") !=
            NULL);
  } else if (strcmp(f.action, "stale-index") == 0) {
    CHECK(umi_file_index_rebuild(umi_studio_services_file_index(
              umi_studio_bootstrap_services(f.bootstrap))) == UMI_STATUS_OK);
    /* This check observes native controls. Preserve the projection-only
     * refresh for review; explicit workspace synchronization publishes the
     * search pane and command availability without waiting for a timer. */
#if 0
    CHECK(umi_studio_gtk_workbench_refresh(f.workbench) == UMI_STATUS_OK);
#endif
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(f.workbench) == UMI_STATUS_OK);
    CHECK(SearchRowCount(&f) == 0U && !gtk_widget_get_sensitive(SearchControl(&f, "next")));
  } else if (strcmp(f.action, "filter-query") == 0) {
    gtk_editable_set_text(GTK_EDITABLE(SearchControl(&f, "query")), "Unsubmitted");
    gtk_editable_set_text(GTK_EDITABLE(SearchControl(&f, "include")), "*.h");
    SearchClick(&f, "next");
    CHECK(strstr(gtk_label_get_text(GTK_LABEL(SearchControl(&f, "scope"))), "PageMarker") != NULL);
    CHECK(SearchRowCount(&f) == 64U);
  } else if (strcmp(f.action, "retired") == 0) {
    GtkWidget *start = g_object_ref(SearchControl(&f, "start")),
              *next = g_object_ref(SearchControl(&f, "next"));
    UmiFileSearchSnapshot before, after;
    CHECK(UmiStudioUiSearchRead(f.ui, &before) == UMI_STATUS_OK);
    umi_studio_gtk_workbench_destroy(f.workbench);
    f.workbench = NULL;
    f.root = NULL;
    g_signal_emit_by_name(start, "clicked");
    g_signal_emit_by_name(next, "clicked");
    CHECK(UmiStudioUiSearchRead(f.ui, &after) == UMI_STATUS_OK &&
          before.requestId == after.requestId);
    g_object_unref(start);
    g_object_unref(next);
  } else if (strcmp(f.action, "invalid-retired") == 0) {
    /* Invalid filters preserve the existing result set, and a retained
     * start control becomes inert after its host is retired. */
    GtkWidget *start = g_object_ref(SearchControl(&f, "start"));
    gtk_editable_set_text(GTK_EDITABLE(SearchControl(&f, "include")), "[unsupported]");

    g_signal_emit_by_name(start, "clicked");
    CHECK(SearchRowCount(&f) == 64U);
    umi_studio_gtk_workbench_destroy(f.workbench);
    f.workbench = NULL;
    f.root = NULL;
    g_signal_emit_by_name(start, "clicked");
    g_object_unref(start);
  } else if (strcmp(f.action, "normal") != 0) {
    const char *part = strcmp(f.action, "summary-close") == 0 ? "summary"
                       : strcmp(f.action, "scope-close") == 0 ? "scope"
                                                              : "range";
    if (strcmp(f.action, "rows-close") == 0) {
      GtkWidget *row = g_object_ref(gtk_widget_get_first_child(SearchControl(&f, "results")));
      g_signal_connect(row, "notify::parent", G_CALLBACK(RowsChanged), &f);
      SearchClick(&f, "next");
      g_object_unref(row);
    } else {
      GtkWidget *observed = g_object_ref(SearchControl(&f, part));
      g_signal_connect(observed, "notify::label", G_CALLBACK(Observed), &f);
      /* Changing the accepted query ensures scope and summary notifications
       * have different values instead of depending on redundant setters. */
      if (strcmp(part, "range") != 0) {
        CHECK(UmiStudioUiSearchStart(f.ui, strcmp(part, "summary") == 0 ? "AbsentMarker" : "Page",
                                     1) == UMI_STATUS_OK);
        SearchWait(&f);
        /* This check observes native controls. Preserve the projection-only
         * refresh for review; explicit workspace synchronization publishes the
         * search pane and command availability without waiting for a timer. */
#if 0
        (void)umi_studio_gtk_workbench_refresh(f.workbench);
#endif
        (void)umi_studio_gtk_workbench_workspace_synchronise(f.workbench);
      } else
        SearchClick(&f, "next");
      g_signal_handlers_disconnect_by_data(observed, &f);
      g_object_unref(observed);
    }
    CHECK(f.callbacks != 0U);
    if (strcmp(f.action, "start-reentry") == 0) {
      UmiFileSearchSnapshot state;
      CHECK(UmiStudioUiSearchRead(f.ui, &state) == UMI_STATUS_OK && !state.active);
    }
  }
  SearchPagesDestroy(&f);
  return 0;
}
