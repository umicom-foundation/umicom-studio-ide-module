/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_saved_ctest_report_gtk4.c
 *
 * PURPOSE:
 *   Verify native saved-report review without modifying Studio live test results.
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
#include "umicom/studio/tests.h"
#include "umicom/ui/gtk4/automation.h"
#include "umicom/ui/gtk4/ctest_report.h"
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
/* Keep the original idle-draining fixture for review. Bounded dispatch lets
 * deadlines advance even when an editor source remains ready continuously. */
#if 0
static void Drain(void) {
  for (unsigned i = 0U; i < 50U; ++i) {
    while (g_main_context_iteration(NULL, FALSE)) {
    }
    g_usleep(1000U);
  }
}
#endif
static void Drain(void) {
  gint64 end = g_get_monotonic_time() + 50 * G_TIME_SPAN_MILLISECOND;
  while (g_get_monotonic_time() < end) {
    for (unsigned i = 0; i < 32U && g_get_monotonic_time() < end; ++i)
      if (!g_main_context_iteration(NULL, FALSE)) break;
    g_usleep(1000U);
  }
}
int main(int argc, char **argv) {
  CHECK(argc == 3);
  const char *name = argv[1], *names[] = {"normal", "filter", "no-project", "retained", "cancel",
      "invalid-filter-preserves", "failed-replacement"};
  bool known = false;
  for (size_t i = 0U; i < sizeof names / sizeof names[0]; ++i)
    known |= !strcmp(name, names[i]);
  CHECK(known);
  if (!gtk_init_check())
    return 77;
  char *directory = g_build_filename(argv[2], "saved-report-XXXXXX", NULL);
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
  if (strcmp(name, "no-project"))
    CHECK(umi_studio_workspace_open(services, directory, 1, 0) == UMI_STATUS_OK);
  char *filename = g_build_filename(directory, "saved.xml", NULL);
  const char xml[] =
      "<testsuite tests='2'><testcase name='first' status='fail' time='0'><failure "
      "message='Failed'/></testcase><testcase name='second' status='run' time='1'/></testsuite>";
  CHECK(g_file_set_contents(filename, xml, sizeof xml - 1U, NULL));
  GtkApplication *app =
      gtk_application_new("org.umicom.StudioSavedReportTest", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(app), NULL, NULL));
  UmiStudioGtkWorkbench *workbench = NULL;
  UmiStudioGtkWorkbenchOptions options = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(app, umi_studio_bootstrap_ui(bootstrap),
                                                     umi_studio_bootstrap_desktop_shell(bootstrap),
                                                     &options, &workbench) == UMI_STATUS_OK);
  /* Request the actual panel before looking up its controls. Workspace hosts
   * create bodies on demand; default layouts are not a fixture dependency. */
  CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_TEST_EXPLORER, NULL) == UMI_STATUS_OK);
  CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
  GtkWindow *window = umi_studio_gtk_workbench_window(workbench);
  gtk_window_present(window);
  GtkWidget *root =
      umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "ctest.report.panel");
  GtkWidget *button =
      umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "ctest.report.open");
  GtkWidget *path = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "ctest.report.path");
  CHECK(root && GTK_IS_BUTTON(button) && GTK_IS_EDITABLE(path));
  g_object_ref(root);
  g_object_ref(button);
  CHECK(umi_gtk4_automation_reveal_tagged_widget(GTK_WIDGET(window), "ctest.report.open") ==
        UMI_STATUS_OK);
  Drain();
  CHECK(gtk_widget_get_mapped(button));
  gtk_editable_set_text(GTK_EDITABLE(path), filename);
  GtkWidget *view = umi_gtk4_automation_find_tagged_widget(root, "ctest.report.output");
  CHECK(GTK_IS_TEXT_VIEW(view));
  GtkTextBuffer *buffer = g_object_ref(gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)));
  if (!strcmp(name, "retained")) {
    umi_studio_gtk_workbench_destroy(workbench);
    workbench = NULL;
  }
  UmiStudioTestSnapshot live_before, live_after;
  CHECK(umi_studio_test_service_snapshot(umi_studio_services_tests(services), &live_before) == UMI_STATUS_OK);
  g_signal_emit_by_name(button, "clicked");
  if (!strcmp(name, "cancel")) {
    GtkWidget *cancel = umi_gtk4_automation_find_tagged_widget(root, "ctest.report.cancel");
    CHECK(GTK_IS_BUTTON(cancel));
    g_signal_emit_by_name(cancel, "clicked");
  }
  gint64 end = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
  while (UmiGtk4CtestReportBusy(root)) {
    CHECK(g_get_monotonic_time() < end);
    g_main_context_iteration(NULL, FALSE);
    g_usleep(1000U);
  }
  GtkTextIter first, last;
  gtk_text_buffer_get_bounds(buffer, &first, &last);
  char *text = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
  if (!strcmp(name, "retained") || !strcmp(name, "cancel"))
    CHECK(!text[0]);
  else
    CHECK(strstr(text, "Showing 1..1 of 1") && strstr(text, "[failed] first") &&
          !strstr(text, "[passed] second"));
  g_free(text);
  if (!strcmp(name, "filter")) {
    GtkWidget *outcome = umi_gtk4_automation_find_tagged_widget(root, "ctest.report.outcome"),
              *apply = umi_gtk4_automation_find_tagged_widget(root, "ctest.report.apply");
    CHECK(GTK_IS_DROP_DOWN(outcome) && GTK_IS_BUTTON(apply));
    gtk_drop_down_set_selected(GTK_DROP_DOWN(outcome), 1U);
    g_signal_emit_by_name(apply, "clicked");
    gtk_text_buffer_get_bounds(buffer, &first, &last);
    text = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
    CHECK(strstr(text, "Showing 1..2 of 2"));
    g_free(text);
  }
  if (!strcmp(name, "invalid-filter-preserves") || !strcmp(name, "failed-replacement")) {
    gtk_text_buffer_get_bounds(buffer, &first, &last);
    char *accepted = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
    if (!strcmp(name, "invalid-filter-preserves")) {
      GtkWidget *outcome = umi_gtk4_automation_find_tagged_widget(root, "ctest.report.outcome");
      GtkWidget *apply = umi_gtk4_automation_find_tagged_widget(root, "ctest.report.apply");
      CHECK(GTK_IS_DROP_DOWN(outcome) && GTK_IS_BUTTON(apply));
      gtk_drop_down_set_selected(GTK_DROP_DOWN(outcome), GTK_INVALID_LIST_POSITION);
      g_signal_emit_by_name(apply, "clicked");
    } else {
      char *missing = g_build_filename(directory, "missing.xml", NULL);
      gtk_editable_set_text(GTK_EDITABLE(path), missing); g_free(missing);
      g_signal_emit_by_name(button, "clicked");
      end = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
      while (UmiGtk4CtestReportBusy(root)) {
        CHECK(g_get_monotonic_time() < end);
        g_main_context_iteration(NULL, FALSE); g_usleep(1000U);
      }
    }
    gtk_text_buffer_get_bounds(buffer, &first, &last);
    text = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
    CHECK(!strcmp(accepted, text)); g_free(accepted); g_free(text);
    GtkWidget *loaded = umi_gtk4_automation_find_tagged_widget(root, "ctest.report.source");
    CHECK(GTK_IS_LABEL(loaded) && !strcmp(gtk_label_get_text(GTK_LABEL(loaded)), filename));
  }
  CHECK(umi_studio_test_service_snapshot(umi_studio_services_tests(services), &live_after) == UMI_STATUS_OK);
  /* Saved external observations must not be inserted into the live runner. */
  CHECK(live_after.test_count == live_before.test_count && live_after.passed == live_before.passed &&
      live_after.failed == live_before.failed && live_after.skipped == live_before.skipped &&
      live_after.cancelled == live_before.cancelled && live_after.timed_out == live_before.timed_out &&
      live_after.selected_count == live_before.selected_count &&
      live_after.retained_result_count == live_before.retained_result_count &&
      live_after.retained_attachment_count == live_before.retained_attachment_count &&
      live_after.operation_running == live_before.operation_running);
  umi_studio_gtk_workbench_destroy(workbench);
  g_object_unref(buffer);
  g_object_unref(button);
  g_object_unref(root);
  g_object_unref(app);
  umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(settings);
  g_free(filename);
  g_free(directory);
  return 0;
}
