/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/editor_config.inc
 *
 * PURPOSE:
 *   Open shared project configuration review using copied workspace identity.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/platform/path.h"
#include "umicom/ui/gtk4/editor_config_inspection.h"
/* Capture names before constructing native widgets. The independent inspector
 * starts no I/O until its user chooses Inspect, and cannot change Studio state. */
static UmiStatus RuntimeOpenEditorConfigInspection(UmiStudioGtkRuntimeChrome *runtime) {
  if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL)
    return UMI_STATUS_INVALID_ARGUMENT;
  UmiStudioWorkspaceSnapshot captured;
  UmiStatus status =
      umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui), &captured);
  if (status != UMI_STATUS_OK)
    return status;
  if (!captured.graph.open || !captured.graph.root[0])
    return UMI_STATUS_NOT_FOUND;
  char relative[UMI_PATH_CAPACITY] = "";
  UmiDocumentWorkingCopySnapshot active;
  if (umi_document_coordinator_active_snapshot(umi_studio_ui_documents(runtime->owner->ui),
                                               &active) == UMI_STATUS_OK &&
      active.has_path && umi_path_is_within(captured.graph.root, active.path)) {
    if (umi_path_relative(captured.graph.root, active.path, relative, sizeof relative) !=
        UMI_STATUS_OK)
      relative[0] = 0;
  }
  UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
  if (!request)
    return UMI_STATUS_OUT_OF_MEMORY;
  GObject *owner = NULL;
  runtime = runtime_file_dialog_owner(request, &owner);
  if (!runtime || !UmiGtk4WindowIsOpen(GTK_WINDOW(owner))) {
    status = UMI_STATUS_CANCELLED;
    goto done;
  }
  if (g_object_get_data(owner, "studio-editor-config-pending")) {
    status = UMI_STATUS_BUSY;
    goto done;
  }
  g_object_set_data(owner, "studio-editor-config-pending", GINT_TO_POINTER(1));
  GtkApplication *app = gtk_window_get_application(GTK_WINDOW(owner));
  if (!app) {
    status = UMI_STATUS_CANCELLED;
    goto completed;
  }
  g_object_ref(app);
  GtkWindow *inspection =
      UmiGtk4EditorConfigInspectionCreateWindow(app, captured.graph.root, relative);
  g_object_unref(app);
  if (!inspection) {
    status = UMI_STATUS_CANCELLED;
    goto completed;
  }
  g_object_ref(inspection);
  GObject *current = NULL;
  runtime = runtime_file_dialog_owner(request, &current);
  if (!runtime || !UmiGtk4WindowIsOpen(GTK_WINDOW(current)))
    status = UMI_STATUS_CANCELLED;
  else {
    UmiStudioWorkspaceSnapshot after;
    status = umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui), &after);
    if (status == UMI_STATUS_OK &&
        (!after.graph.open || after.graph.revision != captured.graph.revision ||
         !umi_path_equal(after.graph.root, captured.graph.root)))
      status = UMI_STATUS_INVALID_STATE;
  }
  g_clear_object(&current);
  if (status == UMI_STATUS_OK)
    gtk_window_present(inspection);
  else
    gtk_window_destroy(inspection);
  g_object_unref(inspection);
completed:
  g_object_set_data(owner, "studio-editor-config-pending", NULL);
done:
  g_clear_object(&owner);
  runtime_file_dialog_request_free(request);
  return status;
}
