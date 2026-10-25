/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/editor_preference_storage.inc
 *
 * PURPOSE:
 *   Run preference I/O on workers with copied values and guarded owner-thread publication.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "editor_preferences_storage.h"
#include "umicom/base/text.h"
#include "umicom/document/indentation_storage.h"
#include "umicom/platform/output_file.h"
typedef struct RuntimePreferenceStorageRequest {
  UmiStudioFileDialogRequest *owner;
  GtkWindow *form;
  GtkWidget *message;
  UmiEditorIndentation reviewed;
  UmiStudioEditorPreferencesStorageSnapshot result;
  int defaultPath;
} RuntimePreferenceStorageRequest;
static void RuntimePreferenceStorageFree(RuntimePreferenceStorageRequest *request) {
  if (request == NULL)
    return;
  runtime_file_dialog_request_free(request->owner);
  g_clear_object(&request->message);
  g_clear_object(&request->form);
  g_free(request);
}
static void RuntimePreferenceStorageWorker(GTask *task, gpointer source, gpointer data,
                                           GCancellable *cancel) {
  (void)source;
  (void)cancel;
  RuntimePreferenceStorageRequest *request = data;
  UmiStatus status = UMI_STATUS_OK;
  if (request->defaultPath)
    status = UmiDocumentIndentationStoragePath("Studio", NULL, request->result.path,
                                               sizeof request->result.path);
  if (status == UMI_STATUS_OK && request->result.save) {
    GFile *file = g_file_new_for_path(request->result.path), *parent = g_file_get_parent(file);
    GError *error = NULL;
    if (parent == NULL || (!g_file_make_directory_with_parents(parent, NULL, &error) &&
                           !g_error_matches(error, G_IO_ERROR, G_IO_ERROR_EXISTS)))
      status = UMI_STATUS_IO_ERROR;
    g_clear_error(&error);
    g_clear_object(&parent);
    g_object_unref(file);
  }
  if (status == UMI_STATUS_OK)
    status =
        request->result.save
            ? UmiDocumentIndentationStorageSave(request->result.path, &request->result.preferences)
            : UmiDocumentIndentationStorageLoad(request->result.path, &request->result.preferences);
  request->result.status = status;
  g_task_return_boolean(task, TRUE);
}
static void RuntimePreferenceStorageDone(GObject *source, GAsyncResult *result, gpointer data) {
  (void)source;
  (void)data;
  RuntimePreferenceStorageRequest *request = g_task_get_task_data(G_TASK(result));
  (void)g_task_propagate_boolean(G_TASK(result), NULL);
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request->owner, &window);
  if (runtime == NULL) {
    /* A captured save may already have completed on disk. Do not relabel
     * that result as cancelled; only publication of a load is cancelled. */
    if (!request->result.save && request->result.status == UMI_STATUS_OK)
      request->result.status = UMI_STATUS_CANCELLED;
  } else if (!request->result.save && request->result.status == UMI_STATUS_OK) {
    request->result.status = UmiStudioUiUpdateEditorPreferences(
        runtime->owner->ui, &request->reviewed, &request->result.preferences);
    if (request->result.status == UMI_STATUS_OK) {
      request->result.applied = 1;
      request->result.status = UmiGtk4AdapterRefreshIndentation(runtime->owner->adapter);
    }
  }
  runtime = RuntimeResolveOwner(request->owner, &window);
  request->result.running = 0;
  /* The retained binding outlives runtime teardown and owns the observation.
   * Clear only this request so a different generation cannot be disturbed. */
  if (g_object_get_data(request->owner->binding, "studio-editor-preference-io") == request) {
    g_object_set_data(request->owner->binding, "studio-editor-preference-io", NULL);
    UmiStudioEditorPreferencesStorageSnapshot *snapshot =
        g_new(UmiStudioEditorPreferencesStorageSnapshot, 1);
    *snapshot = request->result;
    g_object_set_data_full(request->owner->binding, "studio-editor-preference-result", snapshot,
                           g_free);
  }
  if (runtime != NULL) {
    char *message;
    if (request->result.status == UMI_STATUS_OK)
      message = g_strdup_printf(
          request->result.save ? "Saved applied preferences (%u columns, %s) to %s. Later session "
                                 "edits are not part of this save."
                               : "Loaded saved preferences (%u columns, %s) from %s.",
          request->result.preferences.tab_width,
          request->result.preferences.insert_spaces ? "spaces" : "tabs", request->result.path);
    else if (request->result.applied)
      message = g_strdup("Saved preferences reached the session, but native presentation changed "
                         "during refresh. Review the current preferences before continuing.");
    else if (!request->result.save && request->result.status == UMI_STATUS_INVALID_STATE)
      message = g_strdup(
          "Saved preferences were not applied because the session changed during loading.");
    else
      message = g_strdup_printf("Editor preference storage: %s. Location: %s.",
                                umi_status_text(request->result.status), request->result.path);
    if (request->form != NULL && request->message != NULL && UmiGtk4WindowIsOpen(request->form))
      gtk_label_set_text(GTK_LABEL(request->message), message);
    else if (request->result.status != UMI_STATUS_NOT_FOUND || request->result.save)
      runtime_show_notification(runtime, message);
    g_free(message);
  }
  g_clear_object(&window);
  /* Release all GTK-owned state on this completion context, never on a
   * GTask worker that happens to own the last task reference. */
  g_task_set_task_data(G_TASK(result), NULL, NULL);
  RuntimePreferenceStorageFree(request);
}
static UmiStatus RuntimePreferenceStorageStart(UmiStudioGtkRuntimeChrome *runtime, const char *path,
                                               int save, GtkWindow *form, GtkWidget *message) {
  if (runtime == NULL || runtime->owner == NULL || (save != 0 && save != 1))
    return UMI_STATUS_INVALID_ARGUMENT;
  RuntimePreferenceStorageRequest *request = g_new0(RuntimePreferenceStorageRequest, 1);
  request->owner = runtime_file_dialog_request_new(runtime, 0U);
  if (request->owner == NULL) {
    g_free(request);
    return UMI_STATUS_OUT_OF_MEMORY;
  }
  UmiStatus status = UMI_STATUS_OK;
  if (g_object_get_data(request->owner->binding, "studio-editor-preference-io") != NULL)
    status = UMI_STATUS_BUSY;
  if (status == UMI_STATUS_OK && path != NULL) {
    status = UmiOutputFileValidatePath(path);
    if (status == UMI_STATUS_OK)
      status = umi_text_copy(request->result.path, sizeof request->result.path, path);
  }
  if (status == UMI_STATUS_OK)
    status = UmiStudioUiReadEditorPreferences(runtime->owner->ui, &request->reviewed);
  if (status != UMI_STATUS_OK) {
    RuntimePreferenceStorageFree(request);
    return status;
  }
  request->defaultPath = path == NULL;
  request->result.preferences = request->reviewed;
  request->result.save = save;
  request->result.running = 1;
  request->result.status = UMI_STATUS_BUSY;
  if (form != NULL)
    request->form = g_object_ref(form);
  if (message != NULL)
    request->message = g_object_ref(message);
  g_object_set_data(request->owner->binding, "studio-editor-preference-io", request);
  UmiStudioEditorPreferencesStorageSnapshot *snapshot =
      g_new(UmiStudioEditorPreferencesStorageSnapshot, 1);
  *snapshot = request->result;
  g_object_set_data_full(request->owner->binding, "studio-editor-preference-result", snapshot,
                         g_free);
  GTask *task = g_task_new(NULL, NULL, RuntimePreferenceStorageDone, NULL);
  g_task_set_task_data(task, request, NULL);
  g_task_run_in_thread(task, RuntimePreferenceStorageWorker);
  g_object_unref(task);
  return UMI_STATUS_OK;
}
static void RuntimeEditorPreferencesSaveClicked(GtkButton *button, GtkWindow *form) {
  (void)button;
  RuntimeEditorPreferences *state = g_object_get_data(G_OBJECT(form), "studio-editor-preferences");
  if (state == NULL || state->busy || !UmiGtk4WindowIsOpen(form))
    return;
  g_object_ref(form);
  state->busy = 1;
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(state->owner, &window);
  UmiStatus status = UMI_STATUS_CANCELLED;
  UmiEditorIndentation draft, applied;
  if (runtime != NULL) {
    status = UmiGtk4IndentationPreferencesRead(state->controls, &draft);
    if (status == UMI_STATUS_OK)
      status = UmiStudioUiReadEditorPreferences(runtime->owner->ui, &applied);
    if (status == UMI_STATUS_OK &&
        (draft.tab_width != applied.tab_width || draft.insert_spaces != applied.insert_spaces))
      status = UMI_STATUS_INVALID_STATE;
    if (status == UMI_STATUS_OK)
      status = RuntimePreferenceStorageStart(runtime, NULL, 1, form, state->message);
    const char *text = status == UMI_STATUS_OK
                           ? "Saving the applied preferences in your local configuration folder."
                       : status == UMI_STATUS_BUSY
                           ? "Preference storage is busy. Wait for it to finish."
                           : "Apply the valid draft to this session before saving.";
    if (UmiGtk4WindowIsOpen(form))
      gtk_label_set_text(GTK_LABEL(state->message), text);
  }
  state->busy = 0;
  g_clear_object(&window);
  g_object_unref(form);
}
