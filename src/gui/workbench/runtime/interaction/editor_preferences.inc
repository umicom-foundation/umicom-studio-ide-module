/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/editor_preferences.inc
 *
 * PURPOSE:
 *   Present Framework-owned indentation controls with reviewed settings and a weak Studio owner.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/editor_preferences.h"
#include "umicom/ui/gtk4/editor_indentation.h"
#include "umicom/ui/gtk4/indentation_preferences.h"
typedef struct RuntimeEditorPreferences {
  UmiStudioFileDialogRequest *owner;
  GtkWidget *controls, *message;
  UmiEditorIndentation reviewed;
  int busy;
} RuntimeEditorPreferences;
#include "editor_preference_storage.inc"
static void RuntimeEditorPreferencesFree(gpointer data) {
  RuntimeEditorPreferences *state = data;
  runtime_file_dialog_request_free(state->owner);
  g_free(state);
}
static void RuntimeEditorPreferencesClose(GtkButton *button, GtkWindow *form) {
  (void)button;
  gtk_window_destroy(form);
}
static void RuntimeEditorPreferencesApply(GtkButton *button, GtkWindow *form) {
  (void)button;
  RuntimeEditorPreferences *state = g_object_get_data(G_OBJECT(form), "studio-editor-preferences");
  if (state == NULL || state->busy || !UmiGtk4WindowIsOpen(form))
    return;
  g_object_ref(form);
  state->busy = 1;
  UmiEditorIndentation candidate;
  UmiStatus status = UmiGtk4IndentationPreferencesRead(state->controls, &candidate);
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(state->owner, &window);
  int applied = 0;
  if (runtime == NULL)
    status = UMI_STATUS_CANCELLED;
  else if (status == UMI_STATUS_OK) {
    status = UmiStudioUiUpdateEditorPreferences(runtime->owner->ui, &state->reviewed, &candidate);
    if (status == UMI_STATUS_OK) {
      applied = 1;
      state->reviewed = candidate;
      /* Property observers can close the owner. This shared refresh
       * retains native views and detects retirement or a policy change. */
      status = UmiGtk4AdapterRefreshIndentation(runtime->owner->adapter);
    }
  }
  runtime = RuntimeResolveOwner(state->owner, &window);
  if (runtime != NULL && UmiGtk4WindowIsOpen(form)) {
    const char *message =
        applied
            ? (status == UMI_STATUS_OK
                   ? "Applied for this Studio session. Existing source and files were not changed."
                   : "Preferences applied for this session. Native refresh was interrupted; reopen "
                     "the editor to refresh its tab display.")
        : status == UMI_STATUS_INVALID_STATE ? "Preferences changed elsewhere. Close and reopen "
                                               "this form to review the current values."
        : status == UMI_STATUS_BUSY
            ? "Finish the current document operation, then apply again."
            : "Enter a whole tab width from 1 to 16. Preferences were not changed.";
    gtk_label_set_text(GTK_LABEL(state->message), message);
  }
  state->busy = 0;
  g_clear_object(&window);
  g_object_unref(form);
}
static UmiStatus RuntimeOpenEditorPreferences(UmiStudioGtkRuntimeChrome *runtime) {
  if (runtime == NULL || runtime->owner == NULL)
    return UMI_STATUS_INVALID_ARGUMENT;
  UmiEditorIndentation initial;
  UmiStatus status = UmiStudioUiReadEditorPreferences(runtime->owner->ui, &initial);
  if (status != UMI_STATUS_OK)
    return status;
  RuntimeEditorPreferences *state = g_new0(RuntimeEditorPreferences, 1);
  state->owner = runtime_file_dialog_request_new(runtime, 0U);
  state->reviewed = initial;
  if (state->owner == NULL) {
    g_free(state);
    return UMI_STATUS_OUT_OF_MEMORY;
  }
  GObject *window = NULL;
  runtime = runtime_file_dialog_owner(state->owner, &window);
  if (runtime == NULL) {
    RuntimeEditorPreferencesFree(state);
    g_clear_object(&window);
    return UMI_STATUS_CANCELLED;
  }
  /* Do not use the raw runtime after creating the form: toplevel observers
   * may retire the main window while these independent widgets are built. */
  GtkWidget *form = gtk_window_new();
  g_object_ref(form);
  g_object_set_data_full(G_OBJECT(form), "studio-editor-preferences", state,
                         RuntimeEditorPreferencesFree);
  gtk_window_set_title(GTK_WINDOW(form), "Editor Preferences");
  gtk_window_set_transient_for(GTK_WINDOW(form), GTK_WINDOW(window));
  gtk_window_set_destroy_with_parent(GTK_WINDOW(form), TRUE);
  gtk_window_set_default_size(GTK_WINDOW(form), 480, 260);
  (void)umi_gtk4_automation_tag_widget(form, "studio.editor-preferences");
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_set_margin_start(box, 12);
  gtk_widget_set_margin_end(box, 12);
  gtk_widget_set_margin_top(box, 12);
  gtk_widget_set_margin_bottom(box, 12);
  status = UmiGtk4IndentationPreferencesCreate(&initial, &state->controls);
  if (status == UMI_STATUS_OK) {
    (void)umi_gtk4_automation_tag_widget(UmiGtk4IndentationPreferencesWidth(state->controls),
                                         "studio.editor-preferences.width");
    (void)umi_gtk4_automation_tag_widget(UmiGtk4IndentationPreferencesSpaces(state->controls),
                                         "studio.editor-preferences.spaces");
    gtk_box_append(GTK_BOX(box), state->controls);
    /* The former session-only guidance remains for review. Explicit storage now
     * saves the applied pair separately from editing its draft. */
    // state->message = gtk_label_new(
    //         "Changes apply to this session. They do not rewrite source or save a settings file.");
    state->message = gtk_label_new("Apply changes for this session, then Save applied preferences to restore them on normal startup.");
    gtk_label_set_wrap(GTK_LABEL(state->message), TRUE);
    (void)umi_gtk4_automation_tag_widget(state->message, "studio.editor-preferences.message");
    gtk_box_append(GTK_BOX(box), state->message);
    GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *apply = gtk_button_new_with_label("Apply to session");
    GtkWidget *close = gtk_button_new_with_label("Close");
    GtkWidget *save = gtk_button_new_with_label("Save applied preferences");
    (void)umi_gtk4_automation_tag_widget(save, "studio.editor-preferences.save");
    g_signal_connect_object(save, "clicked", G_CALLBACK(RuntimeEditorPreferencesSaveClicked), form, 0);
    (void)umi_gtk4_automation_tag_widget(apply, "studio.editor-preferences.apply");
    (void)umi_gtk4_automation_tag_widget(close, "studio.editor-preferences.close");
    g_signal_connect_object(apply, "clicked", G_CALLBACK(RuntimeEditorPreferencesApply), form, 0);
    g_signal_connect_object(close, "clicked", G_CALLBACK(RuntimeEditorPreferencesClose), form, 0);
    gtk_box_append(GTK_BOX(actions), apply);
    gtk_box_append(GTK_BOX(actions), save);
    gtk_box_append(GTK_BOX(actions), close);
    gtk_box_append(GTK_BOX(box), actions);
  }
  gtk_window_set_child(GTK_WINDOW(form), box);
  runtime = RuntimeResolveOwner(state->owner, &window);
  if (runtime == NULL)
    status = UMI_STATUS_CANCELLED;
  if (status == UMI_STATUS_OK && UmiGtk4WindowIsOpen(GTK_WINDOW(form)))
    gtk_window_present(GTK_WINDOW(form));
  else if (UmiGtk4WindowIsOpen(GTK_WINDOW(form)))
    gtk_window_destroy(GTK_WINDOW(form));
  g_clear_object(&window);
  g_object_unref(form);
  return status;
}
static void RuntimeEditorPreferencesClicked(GtkButton *button, gpointer data) {
  (void)button;
  (void)RuntimeOpenEditorPreferences(data);
}
