/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_source_selection_gtk4.c
 *
 * PURPOSE:
 *   Qualify shared word and line selection commands through the native Studio workbench.
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
#include "umicom/ui/gtk4/source_selection.h"
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
             *names[] = {"word",     "line",        "line-ending", "readonly",         "no-word",
                         "argument", "no-document", "unicode",     "word-then-replace"};
  int known = 0;
  for (size_t i = 0U; i < sizeof names / sizeof names[0]; ++i)
    known |= !strcmp(name, names[i]);
  if (!known)
    return 2;
  if (!gtk_init_check())
    return 77;
  char *directory = g_build_filename(argv[2], "source-selection-XXXXXX", NULL);
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
      gtk_application_new("org.umicom.StudioSourceSelectionTest", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(app), NULL, NULL));
  UmiStudioGtkWorkbench *workbench = NULL;
  UmiStudioGtkWorkbenchOptions options = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(app, ui,
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &options, &workbench) == UMI_STATUS_OK);

  UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
  const char *source = !strcmp(name, "no-word")   ? "  "
                       : !strcmp(name, "unicode") ? "\xc3\xa9"
                                                    "clair beta"
                                                  : "alpha beta\r\nnext";
  char view_id[UMI_UI_ID_CAPACITY] = {0};
  if (strcmp(name, "no-document")) {
    CHECK(umi_document_coordinator_new(documents, "selection.c", view_id, sizeof view_id) ==
          UMI_STATUS_OK);
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, view_id, &view) == UMI_STATUS_OK);
    view.dirty = 1;
    view.read_only = !strcmp(name, "readonly");
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, source, strlen(source)) == UMI_STATUS_OK);
  }
  CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
  const char *command = !strcmp(name, "line")          ? "umicom.document.select-line-content"
                        : !strcmp(name, "line-ending") ? "umicom.document.select-line-with-ending"
                                                       : "umicom.document.select-source-word";
  GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)),
            *entry = Find(root, command);
  CHECK(GTK_IS_BUTTON(entry));
  if (strcmp(name, "no-document")) {
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, view_id, &view) == UMI_STATUS_OK);
    view.cursor_offset = !strcmp(name, "unicode") ? 2U : 1U;
    view.selection_length = 0U;
    CHECK(umi_ui_document_view_model_upsert(views, &view) == UMI_STATUS_OK);
  }
  UmiStatus status = umi_studio_gtk_workbench_execute_command(
      workbench, command, !strcmp(name, "argument") ? "invalid" : NULL);
  if (!strcmp(name, "no-document"))
    CHECK(status == UMI_STATUS_PERMISSION_DENIED);
  else if (!strcmp(name, "argument"))
    CHECK(status == UMI_STATUS_INVALID_ARGUMENT);
  else if (!strcmp(name, "no-word"))
    CHECK(status == UMI_STATUS_NOT_FOUND);
  else {
    CHECK(status == UMI_STATUS_OK);
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views, view_id, &view) == UMI_STATUS_OK);
    size_t expected = !strcmp(name, "line")          ? 10U
                      : !strcmp(name, "line-ending") ? 12U
                      : !strcmp(name, "unicode")     ? 7U
                                                     : 5U;
    CHECK(view.cursor_offset == 0U && view.selection_length == expected);
    char *text = NULL;
    size_t bytes = 0U;
    CHECK(UmiUiDocumentViewModelCopyText(views, view_id, &text, &bytes) == UMI_STATUS_OK &&
          !strcmp(text, source));
    UmiUiDocumentViewModelFreeText(text);
    if (!strcmp(name, "word-then-replace")) {
      GtkWidget *find = Find(root, "studio.editor.find-text"),
                *replace = Find(root, "studio.editor.replace-text");
      CHECK(GTK_IS_EDITABLE(find) && GTK_IS_EDITABLE(replace));
      gtk_editable_set_text(GTK_EDITABLE(find), "alpha");
      gtk_editable_set_text(GTK_EDITABLE(replace), "omega");
      CHECK(umi_studio_gtk_workbench_execute_command(workbench, "editor.replace-selection", NULL) ==
            UMI_STATUS_OK);
      GtkWindow *form = Form();
      CHECK(form != NULL);
      GtkWidget *input = Find(GTK_WIDGET(form), "document.proposal.input");
      CHECK(GTK_IS_TEXT_VIEW(input));
      GtkTextIter first, last;
      GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input));
      gtk_text_buffer_get_bounds(buffer, &first, &last);
      char *proposed = gtk_text_buffer_get_text(buffer, &first, &last, TRUE);
      CHECK(!strcmp(proposed, "omega"));
      g_free(proposed);
      gtk_window_destroy(form);
      g_object_unref(form);
    }
  }
  umi_studio_gtk_workbench_destroy(workbench);
  g_object_unref(app);
  umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(settings);
  g_free(directory);
  return 0;
}
