/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_editor_preferences_gtk4.c
 *
 * PURPOSE:
 *   Exercise the actual editor preferences form, stale input and native owner retirement.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/bootstrap.h"
#include "umicom/studio/editor_preferences.h"
#include "umicom/studio/settings.h"
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
    if (Find(GTK_WIDGET(window), "studio.editor-preferences") != NULL)
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

static void ApplyOnMessage(GObject *label, GParamSpec *property, GtkButton *button) {
  (void)label;
  (void)property;
  g_signal_emit_by_name(button, "clicked");
}

int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  const char *name = argv[1], *cases[] = {"apply",
                                          "tabs",
                                          "cancel",
                                          "invalid",
                                          "stale",
                                          "repeat",
                                          "retained-button",
                                          "retired-owner",
                                          "close-notification",
                                          "owner-during-open",
                                          "reentrant"};
  int known = 0;
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i)
    if (!strcmp(name, cases[i]))
      known = 1;
  if (!known)
    return 2;
  if (!gtk_init_check())
    return 77;
  int failed = 0;
  gboolean changed = FALSE;
  char *cwd = g_get_current_dir(), *oldData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH")),
       *directory = NULL;
  UmiSettings *settings = NULL;
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioGtkWorkbench *workbench = NULL;
  GtkApplication *application = NULL;
  GtkWindow *form = NULL;
  GtkWidget *apply = NULL;
  GError *error = NULL;
  directory = g_dir_make_tmp("umicom-editor-preferences-XXXXXX", &error);
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
  UmiStudioServicesOptions options = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&options, &bootstrap) == UMI_STATUS_OK);
  UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
  application = gtk_application_new("org.umicom.studio.preferences-test", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(application), NULL, &error));
  UmiStudioGtkWorkbenchOptions native = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &native, &workbench) == UMI_STATUS_OK);
  GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
  GtkWidget *button = Find(root, "studio.editor.preferences");
  CHECK(button != NULL);
  CloseOwnerDuringForm closing = {GTK_WINDOW(root), 0};
  gulong closeHandler = 0U;
  if (!strcmp(name, "owner-during-open"))
    closeHandler = g_signal_connect(gtk_window_get_toplevels(), "items-changed",
                                    G_CALLBACK(CloseOwnerOnNewWindow), &closing);
  g_signal_emit_by_name(button, "clicked");
  if (closeHandler != 0U)
    g_signal_handler_disconnect(gtk_window_get_toplevels(), closeHandler);
  Pump();
  if (!strcmp(name, "owner-during-open")) {
    CHECK(closing.retired);
    form = Form();
    CHECK(form == NULL);
    goto cleanup;
  }
  form = Form();
  CHECK(form != NULL);
  GtkWidget *width = Find(GTK_WIDGET(form), "studio.editor-preferences.width");
  GtkWidget *spaces = Find(GTK_WIDGET(form), "studio.editor-preferences.spaces");
  apply = Find(GTK_WIDGET(form), "studio.editor-preferences.apply");
  if (apply != NULL)
    g_object_ref(apply);
  CHECK(width != NULL && spaces != NULL && apply != NULL);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(width), 2.0);
  if (!strcmp(name, "tabs"))
    gtk_check_button_set_active(GTK_CHECK_BUTTON(spaces), FALSE);
  if (!strcmp(name, "invalid"))
    gtk_editable_set_text(GTK_EDITABLE(width), "0");
  if (!strcmp(name, "stale"))
    CHECK(umi_settings_set_integer(
              umi_studio_services_settings(umi_studio_bootstrap_services(bootstrap)),
              UMI_STUDIO_SETTING_EDITOR_TAB_WIDTH, 8) == UMI_STATUS_OK);
  if (!strcmp(name, "cancel") || !strcmp(name, "retained-button"))
    gtk_window_destroy(form);
  if (!strcmp(name, "retired-owner"))
    gtk_window_destroy(GTK_WINDOW(root));
  /* The workbench retains the object, so this verifies native retirement
   * without relying on finalisation to invalidate the callback token. */
  if (!strcmp(name, "retired-owner"))
    CHECK(!UmiGtk4WindowIsOpen(GTK_WINDOW(root)));
  if (!strcmp(name, "close-notification") || !strcmp(name, "reentrant")) {
    GtkWidget *message = Find(GTK_WIDGET(form), "studio.editor-preferences.message");
    CHECK(message != NULL);
    if (!strcmp(name, "close-notification"))
      g_signal_connect_object(message, "notify::label", G_CALLBACK(CloseOnMessage), form, 0);
    else
      g_signal_connect_object(message, "notify::label", G_CALLBACK(ApplyOnMessage), apply, 0);
  }
  if (strcmp(name, "cancel"))
    g_signal_emit_by_name(apply, "clicked");
  Pump();
  UmiEditorIndentation after;
  CHECK(UmiStudioUiReadEditorPreferences(ui, &after) == UMI_STATUS_OK);
  int applied = !strcmp(name, "apply") || !strcmp(name, "tabs") || !strcmp(name, "repeat") ||
                !strcmp(name, "close-notification") || !strcmp(name, "reentrant");
  CHECK(after.tab_width == (applied ? 2U : !strcmp(name, "stale") ? 8U : 4U));
  CHECK(after.insert_spaces == (!strcmp(name, "tabs") ? 0 : 1));
  if (!strcmp(name, "repeat")) {
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(width), 8.0);
    g_signal_emit_by_name(apply, "clicked");
    CHECK(UmiStudioUiReadEditorPreferences(ui, &after) == UMI_STATUS_OK && after.tab_width == 8U);
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
  umi_settings_destroy(settings);
  g_clear_object(&application);
  g_clear_error(&error);
  if (changed && (g_chdir(cwd) != 0))
    failed = 1;
  if (oldData != NULL)
    g_setenv("UMICOM_STUDIO_DATA_PATH", oldData, TRUE);
  else
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  g_free(cwd);
  g_free(oldData);
  g_free(directory);
  return failed;
}
