/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/surfaces/saved_test_report.inc
 *
 * PURPOSE:
 *   Embed Framework saved-report browsing separately from live test results.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/ctest_report.h"
/* Saved external reports remain separate from live Test Explorer results.
 * Framework owns file admission, asynchronous reads and page lifetime. */
static GtkWidget *RuntimeSavedCtestReportCreate(void) {
  GtkWidget *expander = gtk_expander_new("Open a saved CTest report");
  GtkWidget *panel = UmiGtk4CtestReportCreate();
  (void)umi_gtk4_automation_tag_widget(expander, "studio.saved-test-report");
  gtk_expander_set_child(GTK_EXPANDER(expander), panel);
  return expander;
}
