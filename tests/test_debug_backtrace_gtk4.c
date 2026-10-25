/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_debug_backtrace_gtk4.c
 *
 * PURPOSE:
 *   Qualify selected-thread backtrace review through trusted Studio actions and retired controls.
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
static GtkWindow *Review(void) {
  GListModel *windows = gtk_window_get_toplevels();
  for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i) {
    GtkWindow *window = g_list_model_get_item(windows, i);
    if (!strcmp(gtk_widget_get_name(GTK_WIDGET(window)), "debug.backtrace.review") &&
        UmiGtk4WindowIsOpen(window))
      return window;
    g_object_unref(window);
  }
  return NULL;
}
static void Drain(void) {
  for (unsigned i = 0U; i < 50U; ++i) {
    while (g_main_context_iteration(NULL, FALSE)) {
    }
    g_usleep(1000U);
  }
}
int main(int argc, char **argv) {
  CHECK(argc == 3);
  const char *name = argv[1], *names[] = {"normal", "empty",    "running",    "denied",
                                          "hidden", "retained", "owner-close"};
  bool known = false;
  for (size_t i = 0U; i < sizeof names / sizeof names[0]; ++i)
    known |= !strcmp(name, names[i]);
  CHECK(known);
  if (!gtk_init_check())
    return 77;
  char *directory = g_build_filename(argv[2], "backtrace-XXXXXX", NULL);
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
  UmiDebugThreadSnapshot thread = {0};
  strcpy(thread.id, "thread");
  strcpy(thread.name, "fixture");
  thread.current = 1;
  thread.stopped = strcmp(name, "running") != 0;
  CHECK(umi_debug_thread_registry_upsert(umi_debug_service_thread(model), &thread) ==
        UMI_STATUS_OK);
  if (strcmp(name, "empty")) {
    UmiDebugStackFrameSnapshot frame = {0};
    strcpy(frame.id, "frame");
    strcpy(frame.thread_id, thread.id);
    strcpy(frame.name, "sample");
    strcpy(frame.source_uri, "/private/source/sample.c");
    frame.line = 9U;
    CHECK(umi_debug_stack_frame_registry_upsert(umi_debug_service_stack_frame(model), &frame) ==
          UMI_STATUS_OK);
  }
  GtkApplication *app =
      gtk_application_new("org.umicom.StudioBacktraceTest", G_APPLICATION_NON_UNIQUE);
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
      umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "debug.backtrace.capture");
  CHECK(GTK_IS_BUTTON(button));
  g_object_ref(button);
  CHECK(umi_gtk4_automation_reveal_tagged_widget(GTK_WIDGET(window), "debug.backtrace.capture") ==
        UMI_STATUS_OK);
  Drain();
  CHECK(gtk_widget_get_mapped(button));
  if (!strcmp(name, "denied"))
    CHECK(umi_studio_workspace_set_trusted(services, 0) == UMI_STATUS_OK);
  if (!strcmp(name, "hidden"))
    gtk_widget_set_visible(button, FALSE);
  if (!strcmp(name, "retained")) {
    umi_studio_gtk_workbench_destroy(workbench);
    workbench = NULL;
  }
  g_signal_emit_by_name(button, "clicked");
  GtkWindow *review = Review();
  bool expected = !strcmp(name, "normal") || !strcmp(name, "empty") || !strcmp(name, "owner-close");
  CHECK((review != NULL) == expected);
  if (review != NULL) {
    GtkWidget *view =
        umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(review), "debug.backtrace.text");
    CHECK(GTK_IS_TEXT_VIEW(view) && !gtk_text_view_get_editable(GTK_TEXT_VIEW(view)));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    GtkTextIter first, last;
    gtk_text_buffer_get_bounds(buffer, &first, &last);
    char *text = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
    CHECK(strstr(text, "/private") == NULL);
    CHECK(strstr(text, !strcmp(name, "empty") ? "Frames: 0" : "#0 sample") != NULL);
    g_free(text);
    if (!strcmp(name, "owner-close")) {
      umi_studio_gtk_workbench_destroy(workbench);
      workbench = NULL;
      CHECK(!UmiGtk4WindowIsOpen(review));
    }
    gtk_window_destroy(review);
    g_object_unref(review);
  }
  umi_studio_gtk_workbench_destroy(workbench);
  g_object_unref(button);
  g_object_unref(app);
  umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(settings);
  g_free(directory);
  return 0;
}
