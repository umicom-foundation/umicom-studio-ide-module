/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_selection_replace_gtk4.c
 *
 * PURPOSE:
 *   Exercise captured selection replacement through the Studio toolbar and shared review owner.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/ui/gtk4/automation.h"
#include "umicom/ui/gtk4/selection_replace.h"
#include "umicom/ui/gtk4/window_lifecycle.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                                                   \
  do {                                                                                             \
    if (!(x)) {                                                                                    \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);                                      \
      return 1;                                                                                    \
    }                                                                                              \
  } while (0)
static GtkWidget *Find(GtkWidget *root, const char *name) {
  return umi_gtk4_automation_find_tagged_widget(root, name);
}
static GtkWindow *Form(void) {
  GListModel *windows = gtk_window_get_toplevels();
  for (guint i = 0; i < g_list_model_get_n_items(windows); ++i) {
    GtkWindow *window = g_list_model_get_item(windows, i);
    if (!strcmp(gtk_widget_get_name(GTK_WIDGET(window)), "document.proposal.review") &&
        UmiGtk4WindowIsOpen(window))
      return window;
    g_object_unref(window);
  }
  return NULL;
}

int main(int argc, char **argv) {
  if (argc != 3)
    return 2;
  const char *name = argv[1],
             *names[] = {"entry",           "apply",    "undo",        "cancel",      "readonly",
                         "empty-selection", "argument", "no-document", "case-option", "inactive"};
  int known = 0;
  for (size_t i = 0U; i < sizeof names / sizeof names[0]; ++i)
    known |= !strcmp(name, names[i]);
  if (!known)
    return 2;
  if (!gtk_init_check())
    return 77;
  char *directory = g_build_filename(argv[2], "selection-replacement-XXXXXX", NULL);
  CHECK(g_mkdtemp(directory) && g_chdir(directory) == 0);
  g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  CHECK(g_mkdir_with_parents("config", 0700) == 0);
  UmiSettings *settings = NULL;
  CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) ==
        UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
        UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
  CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioServicesOptions services = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&services, &bootstrap) == UMI_STATUS_OK);
  UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
  UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
  GtkApplication *app =
      gtk_application_new("org.umicom.StudioSelectionReplacementTest", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(app), NULL, NULL));
  UmiStudioGtkWorkbench *workbench = NULL;
  UmiStudioGtkWorkbenchOptions options = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(app, ui,
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &options, &workbench) == UMI_STATUS_OK);

  UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
  char view_id[UMI_UI_ID_CAPACITY] = {0};
  UmiDocumentId id = 0U;
  const char *source = !strcmp(name, "case-option") ? "old OLD old" : "old old old";
  if (strcmp(name, "no-document")) {
    CHECK(umi_document_coordinator_new(documents, "selection.c", view_id, sizeof view_id) ==
          UMI_STATUS_OK);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    id = active.document_id;
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, view_id, &view) == UMI_STATUS_OK);
    view.dirty = 1;
    view.read_only = !strcmp(name, "readonly");
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, source, strlen(source)) == UMI_STATUS_OK);
  }
  CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
  GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
  GtkWidget *entry = Find(root, "studio.editor.replace-selection"),
            *find = Find(root, "studio.editor.find-text"),
            *replace = Find(root, "studio.editor.replace-text");
  CHECK(GTK_IS_BUTTON(entry) && GTK_IS_EDITABLE(find) && GTK_IS_EDITABLE(replace));
  gtk_editable_set_text(GTK_EDITABLE(find), "old");
  gtk_editable_set_text(GTK_EDITABLE(replace), "new");
  if (!strcmp(name, "case-option")) {
    GtkWidget *mode = g_object_get_data(G_OBJECT(find), "umicom-search-case");
    CHECK(GTK_IS_DROP_DOWN(mode));
    gtk_drop_down_set_selected(GTK_DROP_DOWN(mode), 1U);
  }
  if (strcmp(name, "no-document")) {
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, view_id, &view) == UMI_STATUS_OK);
    view.cursor_offset = 4U;
    view.selection_length = !strcmp(name, "empty-selection") ? 0U : 3U;
    CHECK(umi_ui_document_view_model_upsert(views, &view) == UMI_STATUS_OK);
  }
  UmiStatus status = umi_studio_gtk_workbench_execute_command(
      workbench, "editor.replace-selection", !strcmp(name, "argument") ? "invalid" : NULL);
  GtkWindow *form = Form();
  if (!strcmp(name, "no-document") || !strcmp(name, "readonly") || !strcmp(name, "empty-selection"))
    CHECK(status == UMI_STATUS_PERMISSION_DENIED && form == NULL);
  else if (!strcmp(name, "argument"))
    CHECK(status == UMI_STATUS_INVALID_ARGUMENT && form == NULL);
  else {
    CHECK(status == UMI_STATUS_OK && form);
    GtkWidget *input = Find(GTK_WIDGET(form), "document.proposal.input");
    CHECK(GTK_IS_TEXT_VIEW(input));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *prefill = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
    CHECK(!strcmp(prefill, !strcmp(name, "case-option") ? "OLD" : "new"));
    g_free(prefill);
    int apply = strcmp(name, "entry") && strcmp(name, "cancel");
    if (!strcmp(name, "inactive"))
      CHECK(umi_document_coordinator_new(documents, "other.c", NULL, 0U) == UMI_STATUS_OK);
    if (!strcmp(name, "cancel"))
      g_signal_emit_by_name(Find(GTK_WIDGET(form), "document.proposal.cancel"), "clicked");
    if (apply) {
      g_signal_emit_by_name(Find(GTK_WIDGET(form), "document.proposal.preview"), "clicked");
      gtk_check_button_set_active(
          GTK_CHECK_BUTTON(Find(GTK_WIDGET(form), "document.proposal.approve")), TRUE);
      g_signal_emit_by_name(Find(GTK_WIDGET(form), "document.proposal.apply"), "clicked");
    }
    if (!strcmp(name, "undo"))
      CHECK(UmiDocumentCoordinatorUndo(documents, id) == UMI_STATUS_OK);
    char *text = NULL;
    size_t bytes = 0U;
    CHECK(UmiUiDocumentViewModelCopyText(views, view_id, &text, &bytes) == UMI_STATUS_OK);
    CHECK(!strcmp(text, apply && strcmp(name, "undo") && strcmp(name, "case-option") ? "old new old"
                                                                                     : source));
    UmiUiDocumentViewModelFreeText(text);
    if (UmiGtk4WindowIsOpen(form))
      gtk_window_destroy(form);
    g_object_unref(form);
  }
  umi_studio_gtk_workbench_destroy(workbench);
  g_object_unref(app);
  umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(settings);
  g_free(directory);
  return 0;
}
