/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_saved_ctest_coverage_gtk4.c
 *
 * PURPOSE:
 *   Verify required host coverage remains separate from Studio live test state.
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
#include "umicom/ui/gtk4/ctest_coverage.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { \
  fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return 1; \
} } while (0)

static void DispatchUntil(gint64 deadline) {
  for (unsigned i = 0; i < 32U && g_get_monotonic_time() < deadline; ++i)
    if (!g_main_context_iteration(NULL, FALSE)) break;
}
static gboolean Wait(GtkWidget *panel) {
  gint64 deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
  while (UmiGtk4CtestCoveragePending(panel) && g_get_monotonic_time() < deadline) {
    DispatchUntil(deadline);
    g_usleep(1000U);
  }
  return !UmiGtk4CtestCoveragePending(panel);
}
static GtkWidget *Find(GtkWidget *root, const char *tag) {
  return umi_gtk4_automation_find_tagged_widget(root, tag);
}
static char *BufferText(GtkTextBuffer *buffer) {
  GtkTextIter first, last;
  gtk_text_buffer_get_bounds(buffer, &first, &last);
  return gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
}
int main(int argc, char **argv) {
  CHECK(argc == 3);
  const char *name = argv[1], *cases[] = {
    "normal", "no-project", "cancel", "retained", "stale", "replacement"
  };
  bool known = false;
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i)
    known |= !strcmp(name, cases[i]);
  CHECK(known);
  if (!gtk_init_check()) return 77;
  char *directory = g_build_filename(argv[2], "saved-coverage-XXXXXX", NULL);
  CHECK(g_mkdtemp(directory) && g_chdir(directory) == 0);
  g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  CHECK(g_mkdir_with_parents("config", 0700) == 0);
  UmiSettings *settings = NULL;
  CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
  CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
  CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioServicesOptions service_options = {0};
  CHECK(umi_studio_bootstrap_create_with_options(&service_options, &bootstrap) == UMI_STATUS_OK);
  UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
  if (strcmp(name, "no-project"))
    CHECK(umi_studio_workspace_open(services, directory, 1, 0) == UMI_STATUS_OK);
  char *filename = g_build_filename(directory, "coverage.csv", NULL);
  char *report = g_build_filename(directory, "observations.xml", NULL);
  const char xml[] =
    "<testsuite tests='2'><testcase name='notes.open' status='run' time='1'/>"
    "<testcase name='notes.save' time='0' status='notrun'><skipped message='SKIP_RETURN_CODE=77'/></testcase></testsuite>";
  const char plan[] =
    "kind,host,configuration,source_identity,test,capability,report\n"
    "host,desktop,Debug,source-A,,,observations.xml\n"
    "require,desktop,Debug,,notes.open,GTK display,\n"
    "require,desktop,Debug,,notes.save,GTK display,\n"
    "require,linux,Debug,,filesystem.exchange,Linux filesystem,\n";
  CHECK(g_file_set_contents(filename, plan, sizeof plan - 1U, NULL));
  CHECK(g_file_set_contents(report, xml, sizeof xml - 1U, NULL));
  GtkApplication *application =
    gtk_application_new("org.umicom.StudioCoverageTest", G_APPLICATION_NON_UNIQUE);
  CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
  UmiStudioGtkWorkbench *workbench = NULL;
  UmiStudioGtkWorkbenchOptions options = {0};
  CHECK(umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
    umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
  /* Request the actual panel before looking up its controls. Workspace hosts
   * create bodies on demand; default layouts are not a fixture dependency. */
  CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_TEST_EXPLORER, NULL) == UMI_STATUS_OK);
  CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
  GtkWindow *window = umi_studio_gtk_workbench_window(workbench);
  gtk_window_present(window);
  GtkWidget *root = Find(GTK_WIDGET(window), "ctest.coverage.panel");
  GtkWidget *open = Find(GTK_WIDGET(window), "ctest.coverage.open");
  CHECK(root && GTK_IS_BUTTON(open));
  g_object_ref(root);
  g_object_ref(open);
  CHECK(umi_gtk4_automation_reveal_tagged_widget(GTK_WIDGET(window), "ctest.coverage.open") == UMI_STATUS_OK);
  gint64 deadline = g_get_monotonic_time() + 50 * G_TIME_SPAN_MILLISECOND;
  while (g_get_monotonic_time() < deadline) { DispatchUntil(deadline); g_usleep(1000U); }
  CHECK(gtk_widget_get_mapped(open));
  GtkWidget *path = Find(root, "ctest.coverage.path");
  GtkWidget *identity = Find(root, "ctest.coverage.identity");
  GtkWidget *view = Find(root, "ctest.coverage.output");
  CHECK(GTK_IS_EDITABLE(path) && GTK_IS_EDITABLE(identity) && GTK_IS_TEXT_VIEW(view));
  gtk_editable_set_text(GTK_EDITABLE(path), filename);
  gtk_editable_set_text(GTK_EDITABLE(identity), !strcmp(name, "stale") ? "source-B" : "source-A");
  GtkTextBuffer *buffer = g_object_ref(gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)));
  UmiStudioTestSnapshot before = {0}, after = {0};
  CHECK(umi_studio_test_service_snapshot(umi_studio_services_tests(services), &before) == UMI_STATUS_OK);
  if (!strcmp(name, "retained")) {
    umi_studio_gtk_workbench_destroy(workbench);
    workbench = NULL;
  }
  g_signal_emit_by_name(open, "clicked");
  if (!strcmp(name, "cancel")) {
    GtkWidget *cancel = Find(root, "ctest.coverage.cancel");
    CHECK(GTK_IS_BUTTON(cancel));
    g_signal_emit_by_name(cancel, "clicked");
  }
  CHECK(Wait(root));
  char *accepted = BufferText(buffer);
  if (!strcmp(name, "retained") || !strcmp(name, "cancel")) {
    CHECK(!accepted[0]);
  } else if (!strcmp(name, "stale")) {
    CHECK(strstr(accepted, "Passed: 0 | Outstanding: 3") &&
      strstr(accepted, "[source mismatch]") && strstr(accepted, "[missing host]"));
  } else {
    CHECK(strstr(accepted, "Passed: 1 | Outstanding: 2") &&
      strstr(accepted, "[passed] desktop / Debug : notes.open") &&
      strstr(accepted, "[skipped] desktop / Debug : notes.save") &&
      strstr(accepted, "[missing host] linux / Debug : filesystem.exchange"));
  }
  if (!strcmp(name, "replacement")) {
    char *missing = g_build_filename(directory, "missing.csv", NULL);
    gtk_editable_set_text(GTK_EDITABLE(path), missing);
    g_free(missing);
    g_signal_emit_by_name(open, "clicked");
    CHECK(Wait(root));
    char *current = BufferText(buffer);
    CHECK(!strcmp(accepted, current));
    g_free(current);
    CHECK(!strcmp(gtk_label_get_text(GTK_LABEL(Find(root, "ctest.coverage.source"))), filename));
  }
  CHECK(umi_studio_test_service_snapshot(umi_studio_services_tests(services), &after) == UMI_STATUS_OK);
  /* Imported observations are review evidence, never a replacement for the
   * current project's discovery, selection, retained results or active job. */
  CHECK(after.test_count == before.test_count && after.passed == before.passed &&
    after.failed == before.failed && after.skipped == before.skipped &&
    after.cancelled == before.cancelled && after.timed_out == before.timed_out &&
    after.selected_count == before.selected_count &&
    after.retained_result_count == before.retained_result_count &&
    after.retained_attachment_count == before.retained_attachment_count &&
    after.operation_running == before.operation_running);
  g_free(accepted);
  umi_studio_gtk_workbench_destroy(workbench);
  g_object_unref(buffer);
  g_object_unref(open);
  g_object_unref(root);
  g_object_unref(application);
  umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(settings);
  g_free(report);
  g_free(filename);
  g_free(directory);
  return 0;
}
