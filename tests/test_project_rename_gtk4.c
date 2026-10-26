/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_project_rename_gtk4.c
 *
 * PURPOSE:
 *   Exercise Explorer rename, stale forms and retained controls through the actual Studio window.
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
#include "umicom/studio/workspace.h"
#include "umicom/ui/gtk4/window_lifecycle.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                                                   \
  do {                                                                                             \
    if (!(x)) {                                                                                    \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);                                      \
      failed = 1;                                                                                  \
      goto cleanup;                                                                                \
    }                                                                                              \
  } while (0)
static GtkWidget *Find(GtkWidget *widget, const char *id) {
  if (widget == NULL)
    return NULL;
  const char *tag = g_object_get_data(G_OBJECT(widget), "umicom-automation-id");
  if (tag != NULL && !strcmp(tag, id))
    return widget;
  for (GtkWidget *child = gtk_widget_get_first_child(widget); child != NULL;
       child = gtk_widget_get_next_sibling(child)) {
    GtkWidget *found = Find(child, id);
    if (found != NULL)
      return found;
  }
  return NULL;
}
static GtkWindow *Form(void) {
  GListModel *windows = gtk_window_get_toplevels();
  for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i) {
    GtkWindow *window = g_list_model_get_item(windows, i);
    if (Find(GTK_WIDGET(window), "studio.project-rename") != NULL)
      return window;
    g_object_unref(window);
  }
  return NULL;
}
static void Pump(void) {
  for (unsigned i = 0U; i < 256U && g_main_context_pending(NULL); ++i)
    (void)g_main_context_iteration(NULL, FALSE);
}
static void CloseOnMessage(GObject *label, GParamSpec *property, GtkWindow *window) {
  (void)label;
  (void)property;
  gtk_window_destroy(window);
}
/* A new toplevel can notify an owner that closes the main window while the
 * form is still being built. The construction must use copied selection only. */
typedef struct CloseOwnerDuringForm {
  GtkWindow *owner;
  int retired;
} CloseOwnerDuringForm;
static void CloseOwnerOnNewWindow(GListModel *model, guint position, guint removed, guint added,
                                  gpointer data) {
  (void)model;
  (void)position;
  (void)removed;
  CloseOwnerDuringForm *state = data;
  if (added != 0U && !state->retired) {
    state->retired = 1;
    gtk_window_destroy(state->owner);
  }
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  const char *test = argv[1];
  const char *cases[] = {"rename",
                         "cancel",
                         "existing",
                         "open",
                         "stale",
                         "retained-button",
                         "retired-owner",
                         "repeat",
                         "close-notification",
                         "owner-during-open"};
  int known = 0;
  for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i)
    if (!strcmp(cases[i], test))
      known = 1;
  if (!known)
    return 2;
  if (!gtk_init_check())
    return 77;
  int failed = 0;
  gboolean changed = FALSE;
  char *cwd = g_get_current_dir(), *oldData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
  char *directory = NULL, *project = NULL, *source = NULL, *destination = NULL, *contents = NULL;
  UmiSettings *settings = NULL;
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioGtkWorkbench *workbench = NULL;
  GtkApplication *application = NULL;
  GtkWindow *form = NULL;
  GtkWidget *apply = NULL;
  GError *error = NULL;
  directory = g_dir_make_tmp("umicom-project-rename-XXXXXX", &error);
  CHECK(directory != NULL && g_chdir(directory) == 0);
  changed = TRUE;
  g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  CHECK(g_mkdir_with_parents("config", 0700) == 0);
  CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) ==
        UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
        UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
  CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
  project = g_build_filename(directory, "project", NULL);
  CHECK(g_mkdir_with_parents(project, 0700) == 0);
  source = g_build_filename(project, "old.c", NULL);
  destination = g_build_filename(project, "new.c", NULL);
  CHECK(g_file_set_contents(source, "source", -1, &error));
  if (!strcmp(test, "existing"))
    CHECK(g_file_set_contents(destination, "existing", -1, &error));
  UmiStudioServicesOptions options = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&options, &bootstrap) == UMI_STATUS_OK);
  UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
  UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
  CHECK(umi_studio_workspace_open(services, project, 1, 0) == UMI_STATUS_OK);
  if (!strcmp(test, "open")) {
    char id[UMI_UI_ID_CAPACITY];
    CHECK(umi_document_coordinator_open(umi_studio_ui_documents(ui), source, id, sizeof id) ==
          UMI_STATUS_OK);
  }
  application = gtk_application_new("org.umicom.studio.rename-test", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(application), NULL, &error));
  UmiStudioGtkWorkbenchOptions native = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &native, &workbench) == UMI_STATUS_OK);
  GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
  /* Explorer starts on a collapsed rail and creates its body on explicit Open.
   * Follow that user action before looking for Rename; revealing a rail must
   * not turn it into a permanently docked panel or edit the saved layout. */
  UmiUiWorkspaceLayout *before = g_new0(UmiUiWorkspaceLayout, 1);
  UmiUiWorkspaceLayout *after = g_new0(UmiUiWorkspaceLayout, 1);
  UmiStatus reveal = umi_studio_gtk_workbench_workspace_snapshot(workbench, before);
  if (reveal == UMI_STATUS_OK)
    reveal = umi_studio_gtk_workbench_workspace_open_surface(
        workbench, UMI_STUDIO_SURFACE_EXPLORER, NULL);
  if (reveal == UMI_STATUS_OK)
    reveal = umi_studio_gtk_workbench_workspace_snapshot(workbench, after);
  int preserved = reveal == UMI_STATUS_OK && memcmp(before, after, sizeof *before) == 0;
  g_free(before);
  g_free(after);
  if (reveal != UMI_STATUS_OK)
    fprintf(stderr, "Explorer reveal before rename: %s\n", umi_status_text(reveal));
  CHECK(preserved);
  /* The former eager lookup is retained as the assertion below. Its setup now
   * includes the explicit rail reveal rather than assuming every body exists. */
  GtkWidget *button = Find(root, "umicom-explorer-rename");
  CHECK(button != NULL);
  CloseOwnerDuringForm closing = {GTK_WINDOW(root), 0};
  gulong closeHandler = 0U;
  if (!strcmp(test, "owner-during-open"))
    closeHandler = g_signal_connect(gtk_window_get_toplevels(), "items-changed",
                                    G_CALLBACK(CloseOwnerOnNewWindow), &closing);
  g_signal_emit_by_name(button, "clicked");
  if (closeHandler != 0U)
    g_signal_handler_disconnect(gtk_window_get_toplevels(), closeHandler);
  Pump();
  if (!strcmp(test, "owner-during-open")) {
    CHECK(closing.retired);
    CHECK(g_file_test(source, G_FILE_TEST_EXISTS) && !g_file_test(destination, G_FILE_TEST_EXISTS));
    form = Form();
    CHECK(form == NULL);
    goto cleanup;
  }
  form = Form();
  CHECK(form != NULL);
  GtkWidget *from = Find(GTK_WIDGET(form), "studio.project-rename.source");
  GtkWidget *name = Find(GTK_WIDGET(form), "studio.project-rename.name");
  apply = Find(GTK_WIDGET(form), "studio.project-rename.apply");
  if (apply != NULL)
    g_object_ref(apply);
  CHECK(from != NULL && name != NULL && apply != NULL);
  gtk_editable_set_text(GTK_EDITABLE(from), "old.c");
  gtk_editable_set_text(GTK_EDITABLE(name), "new.c");
  if (!strcmp(test, "stale"))
    CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
  if (!strcmp(test, "cancel") || !strcmp(test, "retained-button"))
    gtk_window_destroy(form);
  if (!strcmp(test, "retired-owner"))
    gtk_window_destroy(GTK_WINDOW(root));
  if (!strcmp(test, "close-notification")) {
    GtkWidget *message = Find(GTK_WIDGET(form), "studio.project-rename.message");
    CHECK(message != NULL);
    g_signal_connect_object(message, "notify::label", G_CALLBACK(CloseOnMessage), form, 0);
  }
  if (strcmp(test, "cancel"))
    g_signal_emit_by_name(apply, "clicked");
  Pump();
  gboolean renamed =
      !strcmp(test, "rename") || !strcmp(test, "repeat") || !strcmp(test, "close-notification");
  CHECK(g_file_test(source, G_FILE_TEST_EXISTS) == !renamed);
  CHECK(g_file_get_contents(renamed ? destination : source, &contents, NULL, &error));
  CHECK(!strcmp(contents, "source"));
  g_clear_pointer(&contents, g_free);
  if (!strcmp(test, "existing")) {
    CHECK(g_file_get_contents(destination, &contents, NULL, &error));
    CHECK(!strcmp(contents, "existing"));
  }
  if (!strcmp(test, "repeat")) {
    CHECK(g_file_set_contents(source, "second source", -1, &error));
    gtk_editable_set_text(GTK_EDITABLE(name), "second.c");
    g_signal_emit_by_name(apply, "clicked");
    CHECK(g_file_get_contents(source, &contents, NULL, &error));
    CHECK(!strcmp(contents, "second source"));
  }
cleanup:
  if (form != NULL && UmiGtk4WindowIsOpen(form))
    gtk_window_destroy(form);
  g_clear_object(&form);
  g_clear_object(&apply);
  if (workbench != NULL)
    umi_studio_gtk_workbench_destroy(workbench);
  if (bootstrap != NULL)
    umi_studio_bootstrap_destroy(bootstrap);
  if (settings != NULL)
    umi_settings_destroy(settings);
  g_clear_object(&application);
  g_clear_error(&error);
  g_free(contents);
  if (changed)
    (void)g_chdir(cwd);
  if (oldData != NULL)
    (void)g_setenv("UMICOM_STUDIO_DATA_PATH", oldData, TRUE);
  else
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  g_free(oldData);
  g_free(cwd);
  g_free(source);
  g_free(destination);
  g_free(project);
  g_free(directory);
  return failed;
}
