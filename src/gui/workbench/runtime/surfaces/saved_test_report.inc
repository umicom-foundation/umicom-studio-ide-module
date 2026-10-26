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

/* A host requirement is a separate question from browsing one historical run.
 * Framework owns both models, allowing Studio to embed them without inserting
 * external observations into its live test service. */
#include "umicom/ui/gtk4/ctest_coverage.h"
static GtkWidget *RuntimeSavedCtestCoverageCreate(void) {
  GtkWidget *expander = gtk_expander_new("Review required host coverage");
  GtkWidget *panel = UmiGtk4CtestCoverageCreate();
  (void)umi_gtk4_automation_tag_widget(expander, "studio.saved-test-coverage");
  gtk_expander_set_child(GTK_EXPANDER(expander), panel);
  return expander;
}

/* Comparing two saved runs reuses Framework outcome policy. It never changes
 * the current project test discovery, selection or execution state. */
#include "umicom/ui/gtk4/ctest_comparison.h"
static GtkWidget *RuntimeSavedCtestComparisonCreate(void) {
  GtkWidget *expander = gtk_expander_new("Compare two saved test runs");
  GtkWidget *panel = UmiGtk4CtestComparisonCreate();
  (void)umi_gtk4_automation_tag_widget(expander, "studio.saved-test-comparison");
  gtk_expander_set_child(GTK_EXPANDER(expander), panel);
  return expander;
}
