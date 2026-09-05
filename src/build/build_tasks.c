/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/build/build_tasks.c
 *
 * PURPOSE:
 *   Implement the build tasks behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Umicom Studio IDE : OpenSource IDE for developers and Content Creators
 * Repository: https://github.com/umicom-foundation/umicom-studio-ide
 * File: src/build/build_tasks.c
 *
 * PURPOSE:
 *   Orchestrates build/run/test for a given project root.
 *
 * DESIGN:
 *   No direct UI types. Streams process diagnostics through UmiOutputSink and
 *   delegates safe process execution to the shared UmiBuildRunner.
 *
 * API:
 *   umi_build_tasks_new/free/build/run/test/root
 *
 * Created by: Umicom Foundation | Developer: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/

#include <stdarg.h>             /* va_list for emit() helper               */
#include <glib.h>
#include <gio/gio.h>
#include "build_tasks.h"
#include "build_runner.h"
#include "build_system.h"
#include "umi_output_sink.h"

struct _UmiBuildTasks {
  gchar          *root;         /* project root directory (UTF-8)         */
  UmiOutputSink  *sink;         /* where we print user-visible messages   */
  UmiBuildSys    *system;       /* detected commands for this workspace   */
};

/* Emit a simple message to the sink (defensive if sink is NULL). */
static void emit(UmiBuildTasks *t, UmiDiagSeverity sev, const char *fmt, ...) {
  if (!t || !t->sink || !fmt) return;
  va_list ap; va_start(ap, fmt);
  gchar *s = g_strdup_vprintf(fmt, ap);
  va_end(ap);

  UmiDiag d = {0};
  d.severity = sev; d.file = g_strdup(""); d.message = g_strdup(s); d.line = 0; d.column = 0;
  umi_output_sink_emit(t->sink, &d);
  g_free(d.file); g_free(d.message); g_free(s);
}

UmiBuildTasks *umi_build_tasks_new(const char *root, UmiOutputSink *sink) {
  UmiBuildTasks *t = g_new0(UmiBuildTasks, 1);
  t->root = g_strdup(root ? root : ".");
  t->sink = sink;
  t->system = umi_buildsys_detect(t->root);
  emit(t, UMI_DIAG_NOTE, "BuildTasks: initialized for root='%s'", t->root);
  return t;
}

void umi_build_tasks_free(UmiBuildTasks *t) {
  if (!t) return;
  umi_buildsys_free(t->system);
  t->system = NULL;
  g_clear_pointer(&t->root, g_free);
  g_free(t);
}

void umi_build_tasks_set_sink(UmiBuildTasks *t, UmiOutputSink *sink) {
  if (!t) { return; }          /* (fixed misleading indentation warning) */
  t->sink = sink;
}

/* Accessor declared in the header. */
const char *umi_build_tasks_root(const UmiBuildTasks *t) {
  return t ? t->root : NULL;
}

/* Run one detected command in the selected project root without a shell. */
static gboolean run_detected_command(UmiBuildTasks *t,
                                     GPtrArray *command,
                                     GError **error)
{
  const gchar *executable;
  UmiBuildRunner *runner;
  gboolean success;

  if (!t || !t->system || !command || command->len < 2U ||
      g_ptr_array_index(command, 0) == NULL) {
    if (command != NULL) g_ptr_array_free(command, TRUE);
    if (error != NULL) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
                          "no build command is configured");
    }
    return FALSE;
  }
  executable = (const gchar *)g_ptr_array_index(command, 0);
  runner = umi_build_runner_new();
  if (runner == NULL) {
    g_ptr_array_free(command, TRUE);
    if (error != NULL) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NO_SPACE,
                          "could not create the build runner");
    }
    return FALSE;
  }
  umi_build_runner_set_sink(runner, t->sink);
  /* Parenthesised function name bypasses the legacy variadic compatibility
   * macro and calls the canonical six-argument runner API. */
  success = (umi_build_runner_run)(
      runner,
      t->root,
      executable,
      (const gchar * const *)&command->pdata[1],
      NULL,
      FALSE);
  umi_build_runner_free(runner);
  g_ptr_array_free(command, TRUE);
  if (!success && error != NULL) {
    g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_FAILED,
                        "the build command returned a failure status");
  }
  return success;
}

gboolean umi_build_tasks_build(UmiBuildTasks *t, GError **error) {
  if (!t || !t->system) return FALSE;
  return run_detected_command(t, umi_buildsys_build_argv(t->system), error);
}

gboolean umi_build_tasks_run(UmiBuildTasks *t, GError **error) {
  if (!t || !t->system) return FALSE;
  return run_detected_command(t, umi_buildsys_run_argv(t->system), error);
}

gboolean umi_build_tasks_test(UmiBuildTasks *t, GError **error) {
  if (!t || !t->system) return FALSE;
  return run_detected_command(t, umi_buildsys_test_argv(t->system), error);
}

/*  END OF FILE */
