/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/project_rename.inc
 *
 * PURPOSE:
 *   Present closed-entry rename with captured workspace identity and live native owner checks.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/project_rename.h"
typedef struct RuntimeProjectRename {
  UmiStudioFileDialogRequest *owner;
  GtkWidget *source;
  GtkWidget *name;
  GtkWidget *message;
  GtkWidget *apply;
  char root[UMI_PATH_CAPACITY];
  uint64_t revision;
  int busy;
  int completed;
} RuntimeProjectRename;
static void RuntimeProjectRenameFree(gpointer data) {
  RuntimeProjectRename *state = data;
  runtime_file_dialog_request_free(state->owner);
  g_free(state);
}
static void RuntimeProjectRenameCancel(GtkButton *button, GtkWindow *form) {
  (void)button;
  gtk_window_destroy(form);
}
static void RuntimeProjectRenameApply(GtkButton *button, GtkWindow *form) {
  (void)button;
  RuntimeProjectRename *state = g_object_get_data(G_OBJECT(form), "studio-project-rename");
  if (state == NULL || state->busy || state->completed || !UmiGtk4WindowIsOpen(form))
    return;
  g_object_ref(form);
  state->busy = 1;
  char *source = g_strdup(gtk_editable_get_text(GTK_EDITABLE(state->source)));
  char *name = g_strdup(gtk_editable_get_text(GTK_EDITABLE(state->name)));
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(state->owner, &window);
  UmiStatus status = UMI_STATUS_CANCELLED;
  UmiStudioProjectRenameResult result = {0};
  UmiWorkspaceGraphSnapshot workspace;
  if (runtime != NULL &&
      umi_workspace_graph_snapshot(
          umi_studio_services_workspace(umi_studio_ui_services(runtime->owner->ui)), &workspace) ==
          UMI_STATUS_OK &&
      workspace.open && workspace.revision == state->revision &&
      umi_path_equal(workspace.root, state->root))
    status =
        UmiStudioUiRenameProjectEntry(runtime->owner->ui, state->revision, source, name, &result);
  g_free(source);
  g_free(name);
  runtime = RuntimeResolveOwner(state->owner, &window);
  /* Mark completion before any signal-emitting widget update so a retained
   * button or notification observer cannot rename a second entry. */
  state->completed = result.entry.renamed;
  if (runtime != NULL && UmiGtk4WindowIsOpen(form)) {
    char text[512];
    if (result.entry.renamed)
      (void)snprintf(text, sizeof text,
                     "Renamed on disk. %s Build files and source references were not rewritten.",
                     result.refresh_status == UMI_STATUS_OK
                         ? "Explorer refresh requested."
                         : "Use Explorer Refresh to update the file list.");
    else if (status == UMI_STATUS_BUSY)
      (void)snprintf(text, sizeof text,
                     "Close affected source tabs and finish the current build. The workspace must "
                     "remain unchanged.");
    else if (status == UMI_STATUS_ALREADY_EXISTS)
      (void)snprintf(text, sizeof text,
                     "The destination exists or is the same path. Choose another name.");
    else if (status == UMI_STATUS_CANCELLED)
      (void)snprintf(text, sizeof text,
                     "The workspace changed. Close this form and open Rename again.");
    else
      (void)snprintf(text, sizeof text, "Rename rejected: %s. No replacement was performed.",
                     umi_status_text(status));
    gtk_label_set_text(GTK_LABEL(state->message), text);
    if (UmiGtk4WindowIsOpen(form) && state->completed)
      gtk_widget_set_sensitive(state->apply, FALSE);
  }
  state->busy = 0;
  g_clear_object(&window);
  g_object_unref(form);
}
static void RuntimeExplorerRenameInvoked(gpointer widget, UmiStudioGtkRuntimeChrome *runtime) {
  (void)widget;
  if (runtime == NULL || runtime->owner == NULL)
    return;
  UmiWorkspaceGraphSnapshot workspace;
  if (umi_workspace_graph_snapshot(
          umi_studio_services_workspace(umi_studio_ui_services(runtime->owner->ui)), &workspace) !=
          UMI_STATUS_OK ||
      !workspace.open) {
    runtime_show_notification(runtime, "Open a project before renaming its files or folders.");
    return;
  }
  RuntimeProjectRename *state = g_new0(RuntimeProjectRename, 1);
  state->owner = runtime_file_dialog_request_new(runtime, 0U);
  if (state->owner == NULL) {
    g_free(state);
    return;
  }
  memcpy(state->root, workspace.root, sizeof state->root);
  state->revision = workspace.revision;
  GObject *window = NULL;
  runtime = runtime_file_dialog_owner(state->owner, &window);
  if (runtime == NULL) {
    RuntimeProjectRenameFree(state);
    return;
  }
  /* Copy selection before GTK window notifications can retire Studio. */
  char relative[UMI_PATH_CAPACITY] = {0};
  if (runtime->selected_file_path[0] != '\0' &&
      umi_path_is_within(workspace.root, runtime->selected_file_path))
    (void)umi_path_relative(workspace.root, runtime->selected_file_path, relative, sizeof relative);
  GtkWidget *form = gtk_window_new();
  g_object_ref(form);
  gtk_window_set_title(GTK_WINDOW(form), "Rename Project Entry");
  gtk_window_set_transient_for(GTK_WINDOW(form), GTK_WINDOW(window));
  gtk_window_set_destroy_with_parent(GTK_WINDOW(form), TRUE);
  gtk_window_set_default_size(GTK_WINDOW(form), 600, 260);
  (void)umi_gtk4_automation_tag_widget(form, "studio.project-rename");
  g_object_set_data_full(G_OBJECT(form), "studio-project-rename", state, RuntimeProjectRenameFree);
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_set_margin_start(box, 12);
  gtk_widget_set_margin_end(box, 12);
  gtk_widget_set_margin_top(box, 12);
  gtk_widget_set_margin_bottom(box, 12);
  GtkWidget *instructions = gtk_label_new(
      "Rename a closed file or folder in the current project. Close its source tabs first. "
      "The new name stays in the same folder and never replaces an existing entry.");
  gtk_label_set_wrap(GTK_LABEL(instructions), TRUE);
  gtk_box_append(GTK_BOX(box), instructions);
  gtk_box_append(GTK_BOX(box), gtk_label_new("Current path, relative to the project root"));
  state->source = gtk_entry_new();
  (void)umi_gtk4_automation_tag_widget(state->source, "studio.project-rename.source");
  if (relative[0] != '\0')
    gtk_editable_set_text(GTK_EDITABLE(state->source), relative);
  gtk_box_append(GTK_BOX(box), state->source);
  gtk_box_append(GTK_BOX(box), gtk_label_new("New name, without folders"));
  state->name = gtk_entry_new();
  (void)umi_gtk4_automation_tag_widget(state->name, "studio.project-rename.name");
  gtk_box_append(GTK_BOX(box), state->name);
  state->message = gtk_label_new(
      "Build scripts, include paths and source references may need editing afterwards.");
  gtk_label_set_wrap(GTK_LABEL(state->message), TRUE);
  (void)umi_gtk4_automation_tag_widget(state->message, "studio.project-rename.message");
  gtk_box_append(GTK_BOX(box), state->message);
  GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  state->apply = gtk_button_new_with_label("Rename");
  (void)umi_gtk4_automation_tag_widget(state->apply, "studio.project-rename.apply");
  GtkWidget *close = gtk_button_new_with_label("Close");
  g_signal_connect_object(state->apply, "clicked", G_CALLBACK(RuntimeProjectRenameApply), form, 0);
  g_signal_connect_object(close, "clicked", G_CALLBACK(RuntimeProjectRenameCancel), form, 0);
  gtk_box_append(GTK_BOX(actions), state->apply);
  gtk_box_append(GTK_BOX(actions), close);
  gtk_box_append(GTK_BOX(box), actions);
  gtk_window_set_child(GTK_WINDOW(form), box);
  runtime = RuntimeResolveOwner(state->owner, &window);
  if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(form)))
    gtk_window_present(GTK_WINDOW(form));
  else if (UmiGtk4WindowIsOpen(GTK_WINDOW(form)))
    gtk_window_destroy(GTK_WINDOW(form));
  g_clear_object(&window);
  g_object_unref(form);
}
