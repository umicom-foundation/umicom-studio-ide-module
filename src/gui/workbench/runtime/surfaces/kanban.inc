/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/surfaces/kanban.inc
 *
 * PURPOSE:
 *   Present a project board through the shared dock without taking its document or worker
 * ownership.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#if defined(UMICOM_STUDIO_HAS_KANBAN)
#include "umicom/ui/gtk4/kanban_dock.h"
typedef struct StudioBoardPane {
  GtkWidget *root, *status, *dock;
  UmiStudioFileDialogRequest *owner;
  bool changing;
} StudioBoardPane;
typedef struct StudioBoardPaneBinding {
  GWeakRef root;
} StudioBoardPaneBinding;
static void RuntimeBoardPaneBindingFree(gpointer data, GClosure *closure) {
  (void)closure;
  StudioBoardPaneBinding *binding = data;
  g_weak_ref_clear(&binding->root);
  g_free(binding);
}
static void RuntimeBoardPaneFree(gpointer data) {
  StudioBoardPane *state = data;
  /* Framework restores the independent window when this pane is released.
   * The Studio workspace never takes ownership of the board's file lease. */
  if (state->dock != NULL) {
    (void)UmiKanbanGtk4Undock(state->dock, true);
    g_object_unref(state->dock);
  }
  runtime_file_dialog_request_free(state->owner);
  g_object_unref(state->status);
  g_free(state);
}
static void RuntimeBoardPaneOpen(GtkButton *button, gpointer data) {
  (void)button;
  StudioBoardPaneBinding *binding = data;
  GtkWidget *root = g_weak_ref_get(&binding->root);
  if (root == NULL)
    return;
  StudioBoardPane *state = g_object_get_data(G_OBJECT(root), "studio-board-pane");
  if (state == NULL || state->changing) {
    g_object_unref(root);
    return;
  }
  state->changing = true;
  GObject *owner = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(state->owner, &owner);
  UmiStatus status = UMI_STATUS_CANCELLED;
  GtkWindow *board = NULL;
  if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(owner)))
    status = RuntimeAcquireProjectBoard(runtime, &board);
  if (status == UMI_STATUS_OK && state->dock != NULL) {
    UmiStatus returned = UmiKanbanGtk4Undock(state->dock, true);
    if (returned == UMI_STATUS_BUSY)
      status = returned;
    else {
      GtkWidget *old = state->dock;
      state->dock = NULL;
      if (gtk_widget_get_parent(old) == root)
        gtk_box_remove(GTK_BOX(root), old);
      g_object_unref(old);
    }
  }
  if (status == UMI_STATUS_OK && !UmiGtk4WindowIsOpen(GTK_WINDOW(owner)))
    status = UMI_STATUS_CANCELLED;
  GtkWidget *candidate = NULL;
  if (status == UMI_STATUS_OK)
    status = UmiKanbanGtk4DockWindow(GTK_WINDOW(owner), board, &candidate);
  if (status == UMI_STATUS_OK) {
    state->dock = candidate;
    gtk_widget_set_vexpand(candidate, TRUE);
    gtk_widget_set_hexpand(candidate, TRUE);
    if (UmiGtk4WindowIsOpen(GTK_WINDOW(owner)))
      gtk_box_append(GTK_BOX(root), candidate);
    else {
      (void)UmiKanbanGtk4Undock(candidate, true);
      status = UMI_STATUS_CANCELLED;
    }
  }
  if (UmiGtk4WindowIsOpen(GTK_WINDOW(owner))) {
    UmiKanbanProjectLocation location;
    if (status == UMI_STATUS_OK &&
        UmiKanbanGtk4ProjectLocation(board, &location) == UMI_STATUS_OK) {
      char *text = g_strdup_printf("Board project: %s\nSwitching the Studio project does not "
                                   "retarget this board. Use Dock current project's board to "
                                   "choose another; the previous board remains in its own window.",
                                   location.project_root);
      gtk_label_set_text(GTK_LABEL(state->status), text);
      g_free(text);
    } else {
      char *text =
          g_strdup_printf("Cannot dock the project board: %s. Open a project first; finish pending "
                          "board file work or return a board already docked in another host.",
                          umi_status_text(status));
      gtk_label_set_text(GTK_LABEL(state->status), text);
      g_free(text);
    }
  }
  g_clear_object(&board);
  g_clear_object(&owner);
  state->changing = false;
  g_object_unref(root);
}
#endif
static GtkWidget *runtime_build_surface_kanban(UmiStudioGtkRuntimeChrome *runtime) {
  GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_set_name(root, "studio-project-board-pane");
#if defined(UMICOM_STUDIO_HAS_KANBAN)
  StudioBoardPane *state = g_new0(StudioBoardPane, 1);
  state->root = root;
  state->owner = runtime_file_dialog_request_new(runtime, 0U);
  if (state->owner == NULL) {
    g_free(state);
    gtk_box_append(GTK_BOX(root), gtk_label_new("Project board owner unavailable."));
    return root;
  }
  state->status = gtk_label_new("Dock a board for the current project. Choose private storage "
                                "explicitly; opening this pane performs no file operations.");
  g_object_ref_sink(state->status);
  gtk_label_set_wrap(GTK_LABEL(state->status), TRUE);
  gtk_widget_set_name(state->status, "studio-board-pane-status");
  gtk_box_append(GTK_BOX(root), state->status);
  GtkWidget *button = gtk_button_new_with_label("Dock current project's board");
  gtk_widget_set_name(button, "studio-board-pane-open");
  StudioBoardPaneBinding *binding = g_new0(StudioBoardPaneBinding, 1);
  g_weak_ref_init(&binding->root, G_OBJECT(root));
  g_signal_connect_data(button, "clicked", G_CALLBACK(RuntimeBoardPaneOpen), binding,
                        RuntimeBoardPaneBindingFree, 0);
  gtk_box_append(GTK_BOX(root), button);
  g_object_set_data_full(G_OBJECT(root), "studio-board-pane", state, RuntimeBoardPaneFree);
#else
  (void)runtime;
  gtk_box_append(GTK_BOX(root),
                 gtk_label_new("This build does not include native Kanban support."));
#endif
  return root;
}
