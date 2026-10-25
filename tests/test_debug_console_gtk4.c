/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_debug_console_gtk4.c
 *
 * PURPOSE:
 *   Exercise Framework console filtering and captured-page ownership through Studio workspace
 * guards.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/bootstrap.h"
#include "umicom/studio/debugger.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/ui/gtk4/automation.h"
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
static void Drain(void) {
  for (unsigned i = 0U; i < 50U; ++i) {
    while (g_main_context_iteration(NULL, FALSE)) {
    }
    g_usleep(1000U);
  }
}
int main(int argc, char **argv) {
  CHECK(argc == 3);
  const char *name = argv[1], *names[] = {"normal", "filter", "denied", "retained", "clear"};
  bool known = false;
  for (size_t i = 0U; i < sizeof names / sizeof names[0]; ++i)
    known |= !strcmp(name, names[i]);
  CHECK(known);
  if (!gtk_init_check())
    return 77;
  char *directory = g_build_filename(argv[2], "console-XXXXXX", NULL);
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
  UmiStudioServicesOptions services_options = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&services_options, &bootstrap) == UMI_STATUS_OK);
  UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
  CHECK(umi_studio_workspace_open(services, directory, 1, 0) == UMI_STATUS_OK);
  UmiDebugService *model =
      umi_studio_debugger_service_model(umi_studio_services_debugger(services));
  for (size_t i = 0U; i < 150U; ++i) {
    UmiDebugConsoleEntrySnapshot entry = {0};
    (void)snprintf(entry.id, sizeof entry.id, "entry-%04zu", i);
    (void)snprintf(entry.text, sizeof entry.text, "output-%04zu", i);
    strcpy(entry.category, i % 2U ? "stderr" : "stdout");
    entry.timestamp = i;
    entry.severity = i % 2U ? 3 : 0;
    CHECK(umi_debug_console_entry_registry_upsert(umi_debug_service_console_entry(model), &entry) ==
          UMI_STATUS_OK);
  }
  GtkApplication *app =
      gtk_application_new("org.umicom.StudioConsoleTest", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(app), NULL, NULL));
  UmiStudioGtkWorkbench *workbench = NULL;
  UmiStudioGtkWorkbenchOptions options = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(app, umi_studio_bootstrap_ui(bootstrap),
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &options, &workbench) == UMI_STATUS_OK);
  CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
  GtkWindow *window = umi_studio_gtk_workbench_window(workbench);
  gtk_window_present(window);
  GtkWidget *button =
      umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "debug.console.refresh");
  GtkWidget *view =
      umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "debug.console.output");
  CHECK(GTK_IS_BUTTON(button) && GTK_IS_TEXT_VIEW(view));
  g_object_ref(button);
  GtkTextBuffer *buffer = g_object_ref(gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)));
  CHECK(umi_gtk4_automation_reveal_tagged_widget(GTK_WIDGET(window), "debug.console.refresh") ==
        UMI_STATUS_OK);
  Drain();
  CHECK(gtk_widget_get_mapped(button));
  if (!strcmp(name, "denied"))
    CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
  if (!strcmp(name, "retained")) {
    umi_studio_gtk_workbench_destroy(workbench);
    workbench = NULL;
  }
  if (!strcmp(name, "filter")) {
    GtkWidget *filter =
        umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "debug.console.filter");
    CHECK(GTK_IS_EDITABLE(filter));
    gtk_editable_set_text(GTK_EDITABLE(filter), "output-0149");
  }
  g_signal_emit_by_name(button, "clicked");
  GtkTextIter first, last;
  gtk_text_buffer_get_bounds(buffer, &first, &last);
  char *text = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
  if (!strcmp(name, "denied") || !strcmp(name, "retained"))
    CHECK(text[0] == '\0');
  else if (!strcmp(name, "filter"))
    CHECK(strstr(text, "Showing 1..1 of 1") && strstr(text, "output-0149"));
  else
    CHECK(strstr(text, "Showing 1..64 of 150"));
  g_free(text);
  if (!strcmp(name, "normal")) {
    umi_debug_console_entry_registry_clear(umi_debug_service_console_entry(model));
    GtkWidget *next =
        umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "debug.console.next");
    CHECK(GTK_IS_BUTTON(next));
    g_signal_emit_by_name(next, "clicked");
    gtk_text_buffer_get_bounds(buffer, &first, &last);
    text = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
    CHECK(strstr(text, "Showing 65..128 of 150"));
    g_free(text);
  }
  if (!strcmp(name, "clear")) {
    GtkWidget *clear =
        umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "debug.console.clear");
    CHECK(GTK_IS_BUTTON(clear));
    g_signal_emit_by_name(clear, "clicked");
    gtk_text_buffer_get_bounds(buffer, &first, &last);
    text = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
    CHECK(text[0] == '\0');
    g_free(text);
    CHECK(umi_debug_console_entry_registry_count(umi_debug_service_console_entry(model)) == 150U);
  }
  umi_studio_gtk_workbench_destroy(workbench);
  g_object_unref(buffer);
  g_object_unref(button);
  g_object_unref(app);
  umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(settings);
  g_free(directory);
  return 0;
}
