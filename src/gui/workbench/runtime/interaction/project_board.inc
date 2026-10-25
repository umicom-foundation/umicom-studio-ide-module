/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/project_board.inc
 *
 * PURPOSE:
 *   Launch or return to the board belonging to a captured workspace through shared Framework
 * services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#if defined(UMICOM_STUDIO_HAS_KANBAN)
#include "umicom/ui/gtk4/kanban_project.h"
#include "umicom/ui/gtk4/kanban_binding.h"
/* A project board is an independent document window. Capture the workspace
 * before native notifications; a later project switch must not retarget it. */
/* Preserve the former direct presenter for review. Acquisition now returns
 * one referenced project window so pane and window entry share identical
 * identity checks, reuse policy and document ownership. */
#if 0
static UmiStatus RuntimeOpenProjectBoard(UmiStudioGtkRuntimeChrome *runtime) {
  if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL)
    return UMI_STATUS_INVALID_ARGUMENT;
  UmiStudioWorkspaceSnapshot captured;
  UmiStatus status =
      umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui), &captured);
  if (status != UMI_STATUS_OK)
    return status;
  if (!captured.graph.open || captured.graph.root[0] == '\0')
    return UMI_STATUS_NOT_FOUND;
  UmiKanbanProjectLocation location;
  status = UmiKanbanProjectLocationCreate(captured.graph.root, &location);
  if (status != UMI_STATUS_OK)
    return status;
  UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
  if (request == NULL)
    return UMI_STATUS_OUT_OF_MEMORY;
  GObject *owner = NULL;
  runtime = runtime_file_dialog_owner(request, &owner);
  GtkWindow *board = NULL;
  int created = 0;
  if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(owner))) {
    status = UMI_STATUS_CANCELLED;
    goto done;
  }
  if (g_object_get_data(owner, "studio-project-board-pending") != NULL) {
    status = UMI_STATUS_BUSY;
    goto done;
  }
  g_object_set_data(owner, "studio-project-board-pending", GINT_TO_POINTER(1));
  GtkApplication *application = gtk_window_get_application(GTK_WINDOW(owner));
  if (application == NULL) {
    status = UMI_STATUS_CANCELLED;
    goto completed;
  }
  g_object_ref(application);
  for (GList *item = gtk_application_get_windows(application); item != NULL; item = item->next) {
    UmiKanbanProjectLocation existing;
    if (UmiKanbanGtk4ProjectLocation(GTK_WINDOW(item->data), &existing) == UMI_STATUS_OK &&
        strcmp(existing.project_root, location.project_root) == 0) {
      board = g_object_ref(item->data);
      break;
    }
  }
  if (board == NULL) {
    status = UmiKanbanGtk4CreateProjectWindow(application, location.project_root, &board);
    if (status == UMI_STATUS_OK) {
      g_object_ref(board);
      created = 1;
      /* Framework owns private location persistence and worker lifetime.
       * Failure leaves the existing manual Open/Create workflow available;
       * the board window reports unavailable settings without losing edits. */
      (void)UmiKanbanGtk4EnableProjectStorageForApplication(board, "Studio", NULL);
    }
  }
  g_object_unref(application);
  if (status == UMI_STATUS_OK) {
    GObject *current = NULL;
    runtime = runtime_file_dialog_owner(request, &current);
    UmiStudioWorkspaceSnapshot after;
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(current)))
      status = UMI_STATUS_CANCELLED;
    else {
      status = umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui), &after);
      if (status == UMI_STATUS_OK &&
          (!after.graph.open || after.graph.revision != captured.graph.revision ||
           !umi_path_equal(after.graph.root, captured.graph.root)))
        status = UMI_STATUS_INVALID_STATE;
    }
    g_clear_object(&current);
  }
  if (status == UMI_STATUS_OK)
    gtk_window_present(board);
  else if (created)
    gtk_window_destroy(board);
  g_clear_object(&board);
completed:
  g_object_set_data(owner, "studio-project-board-pending", NULL);
done:
  g_clear_object(&owner);
  runtime_file_dialog_request_free(request);
  return status;
}
#endif
#include "umicom/ui/gtk4/kanban_dock.h"
static UmiStatus RuntimeAcquireProjectBoard(UmiStudioGtkRuntimeChrome *runtime, GtkWindow **out) {
  if (out == NULL) return UMI_STATUS_INVALID_ARGUMENT;
  *out = NULL;
  if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL)
    return UMI_STATUS_INVALID_ARGUMENT;
  UmiStudioWorkspaceSnapshot captured;
  UmiStatus status =
      umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui), &captured);
  if (status != UMI_STATUS_OK)
    return status;
  if (!captured.graph.open || captured.graph.root[0] == '\0')
    return UMI_STATUS_NOT_FOUND;
  UmiKanbanProjectLocation location;
  status = UmiKanbanProjectLocationCreate(captured.graph.root, &location);
  if (status != UMI_STATUS_OK)
    return status;
  UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
  if (request == NULL)
    return UMI_STATUS_OUT_OF_MEMORY;
  GObject *owner = NULL;
  runtime = runtime_file_dialog_owner(request, &owner);
  GtkWindow *board = NULL;
  int created = 0;
  if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(owner))) {
    status = UMI_STATUS_CANCELLED;
    goto done;
  }
  if (g_object_get_data(owner, "studio-project-board-pending") != NULL) {
    status = UMI_STATUS_BUSY;
    goto done;
  }
  g_object_set_data(owner, "studio-project-board-pending", GINT_TO_POINTER(1));
  GtkApplication *application = gtk_window_get_application(GTK_WINDOW(owner));
  if (application == NULL) {
    status = UMI_STATUS_CANCELLED;
    goto completed;
  }
  g_object_ref(application);
  for (GList *item = gtk_application_get_windows(application); item != NULL; item = item->next) {
    UmiKanbanProjectLocation existing;
    if (UmiKanbanGtk4ProjectLocation(GTK_WINDOW(item->data), &existing) == UMI_STATUS_OK &&
        strcmp(existing.project_root, location.project_root) == 0) {
      board = g_object_ref(item->data);
      break;
    }
  }
  if (board == NULL) {
    status = UmiKanbanGtk4CreateProjectWindow(application, location.project_root, &board);
    if (status == UMI_STATUS_OK) {
      g_object_ref(board);
      created = 1;
      /* Framework owns private location persistence and worker lifetime.
       * Failure leaves the existing manual Open/Create workflow available;
       * the board window reports unavailable settings without losing edits. */
      (void)UmiKanbanGtk4EnableProjectStorageForApplication(board, "Studio", NULL);
    }
  }
  g_object_unref(application);
  if (status == UMI_STATUS_OK) {
    GObject *current = NULL;
    runtime = runtime_file_dialog_owner(request, &current);
    UmiStudioWorkspaceSnapshot after;
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(current)))
      status = UMI_STATUS_CANCELLED;
    else {
      status = umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui), &after);
      if (status == UMI_STATUS_OK &&
          (!after.graph.open || after.graph.revision != captured.graph.revision ||
           !umi_path_equal(after.graph.root, captured.graph.root)))
        status = UMI_STATUS_INVALID_STATE;
    }
    g_clear_object(&current);
  }
  if (status == UMI_STATUS_OK)
    { *out = board; board = NULL; }
  else if (created)
    gtk_window_destroy(board);
  g_clear_object(&board);
completed:
  g_object_set_data(owner, "studio-project-board-pending", NULL);
done:
  g_clear_object(&owner);
  runtime_file_dialog_request_free(request);
  return status;
}
static UmiStatus RuntimeOpenProjectBoard(UmiStudioGtkRuntimeChrome *runtime) {
    GtkWindow *board=NULL;
    UmiStatus status=RuntimeAcquireProjectBoard(runtime,&board);
    if(status==UMI_STATUS_OK)status=UmiKanbanGtk4PresentWindow(board);
    g_clear_object(&board);return status;
}

#endif
