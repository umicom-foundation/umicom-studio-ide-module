/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_console.inc
 *
 * PURPOSE:
 *   Query the current trusted debugger model through Framework-owned console captures.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/debug_console.h"
static UmiStatus RuntimeDebugConsoleQuery(void *context, const UmiDebugConsoleFilter *filter,
                                          UmiDebugConsoleReport **out) {
  if (out == NULL)
    return UMI_STATUS_INVALID_ARGUMENT;
  *out = NULL;
  StudioDebugWorkspaceBinding *binding = context;
  GObject *window = NULL;
  UmiStudioServices *services = NULL;
  UmiStudioWorkspaceSnapshot workspace;
  UmiStatus status = RuntimeDebugWorkspaceOwner(binding, &window, &services, &workspace);
  UmiDebugConsoleQuery *query = NULL;
  if (status == UMI_STATUS_OK)
    status = umi_debug_console_query_create(&query);
  if (status == UMI_STATUS_OK) {
    UmiDebugService *model =
        umi_studio_debugger_service_model(umi_studio_services_debugger(services));
    status = umi_debug_console_query_execute(query, umi_debug_service_console_entry(model), filter);
  }
  if (status == UMI_STATUS_OK)
    status = UmiDebugConsoleReportCreate(query, out);
  umi_debug_console_query_destroy(query);
  g_clear_object(&window);
  return status;
}
static GtkWidget *RuntimeDebugConsoleCreate(UmiStudioGtkRuntimeChrome *runtime) {
  StudioDebugWorkspaceBinding *binding = g_new0(StudioDebugWorkspaceBinding, 1);
  binding->owner = runtime_file_dialog_request_new(runtime, 0U);
  if (binding->owner == NULL) {
    g_free(binding);
    return gtk_label_new("Debugger output is unavailable.");
  }
  GtkWidget *panel = UmiGtk4DebugConsoleCreate(RuntimeDebugConsoleQuery, binding,
                                               RuntimeDebugWorkspaceBindingFree);
  if (panel == NULL) {
    RuntimeDebugWorkspaceBindingFree(binding);
    return gtk_label_new("Debugger output is unavailable.");
  }
  GtkWidget *expander = gtk_expander_new("Debugger output");
  gtk_expander_set_child(GTK_EXPANDER(expander), panel);
  return expander;
}
