/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_backtrace.inc
 *
 * PURPOSE:
 *   Capture selected-thread observations through shared Framework ownership and review.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/debug_backtrace.h"
static void RuntimeDebugBacktraceClicked(GtkButton *button, gpointer context) {
  /* A retained control must not create a review after becoming hidden or
   * unavailable. Keep the original unused-argument marker for comparison;
   * the weak owner binding still governs the window lifetime separately. */
#if 0
  (void)button;
#endif
  if (!gtk_widget_get_mapped(GTK_WIDGET(button)) ||
      !gtk_widget_is_sensitive(GTK_WIDGET(button)))
    return;
  UmiStudioGtkRuntimeChrome *runtime = context;
  if (runtime == NULL || runtime->owner == NULL)
    return;
  UmiStudioFileDialogRequest *owner = runtime_file_dialog_request_new(runtime, 0U);
  if (owner == NULL)
    return;
  StudioDebugWorkspaceBinding binding = {0};
  binding.owner = owner;
  GObject *window = NULL;
  UmiStudioServices *services = NULL;
  UmiStudioWorkspaceSnapshot workspace;
  UmiStatus status = RuntimeDebugWorkspaceOwner(&binding, &window, &services, &workspace);
  UmiDebugBacktrace *trace = NULL;
  if (status == UMI_STATUS_OK) {
    UmiStudioDebuggerService *debugger = umi_studio_services_debugger(services);
    status = UmiDebugBacktraceCapture(umi_studio_debugger_service_workspace(debugger), &trace);
  }
  GtkWindow *review = NULL;
  if (status == UMI_STATUS_OK)
    status = UmiGtk4DebugBacktraceCreateWindow(GTK_WINDOW(window), trace, &review);
  UmiDebugBacktraceDestroy(trace);
  if (status == UMI_STATUS_OK) {
    /* The review owns a cloned observation; it never retains the Studio
     * debugger or a raw project pointer after this action returns. */
    g_object_ref(review);
    if (UmiGtk4WindowIsOpen(review))
      gtk_window_present(review);
    g_object_unref(review);
  } else {
    GObject *current = NULL;
    runtime = runtime_file_dialog_owner(owner, &current);
    if (runtime != NULL)
      runtime_show_notification(
          runtime,
          status == UMI_STATUS_NOT_FOUND || status == UMI_STATUS_INVALID_STATE
              ? "Select a stopped thread with retained stack frames, then capture its call stack."
              : umi_status_text(status));
    g_clear_object(&current);
  }
  g_clear_object(&window);
  runtime_file_dialog_request_free(owner);
}
static GtkWidget *RuntimeDebugBacktraceCreate(UmiStudioGtkRuntimeChrome *runtime) {
  GtkWidget *button = gtk_button_new_with_label("Review captured call stack");
  (void)umi_gtk4_automation_tag_widget(button, "debug.backtrace.capture");
  runtime_bind_editor_action(runtime, button, RuntimeDebugBacktraceClicked);
  return button;
}
