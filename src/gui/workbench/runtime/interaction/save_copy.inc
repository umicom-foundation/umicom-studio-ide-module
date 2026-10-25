/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/save_copy.inc
 *
 * PURPOSE:
 *   Compose Save Copy through the shared immutable draft and exclusive output services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/document/copy.h"

typedef struct StudioSaveCopyRequest {
  UmiStudioFileDialogRequest *owner;
  UmiDocumentCopy *copy;
} StudioSaveCopyRequest;

static void StudioSaveCopyFree(StudioSaveCopyRequest *request) {
  if (request == NULL)
    return;
  runtime_file_dialog_request_free(request->owner);
  UmiDocumentCopyDestroy(request->copy);
  g_free(request);
}

/* Results never retarget the document. A failed write may have left a new file;
 * report that distinction instead of encouraging a blind overwrite retry. */
static UmiStatus StudioSaveCopyWrite(StudioSaveCopyRequest *request, const char *path) {
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request->owner, &window);
  UmiStatus status = UMI_STATUS_CANCELLED;
  if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window))) {
    UmiDocumentCopyResult result = {0};
    status = UmiDocumentCopyWriteNew(request->copy, path, &result);
    /* No product callback executes during synchronous disk output, but use
     * the same generation token before publishing native feedback. */
    runtime = RuntimeResolveOwner(request->owner, &window);
    if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window))) {
      char message[320];
      if (status == UMI_STATUS_OK)
        (void)snprintf(
            message, sizeof message,
            "Copy saved. The source name, unsaved changes and Undo history are unchanged.");
      else if (status == UMI_STATUS_ALREADY_EXISTS)
        (void)snprintf(message, sizeof message,
                       "That file already exists. Save Copy needs a new filename; the existing "
                       "file was kept.");
      else if (result.created)
        (void)snprintf(message, sizeof message,
                       "Copy failed: %s. A new file may be incomplete; inspect it before retrying.",
                       umi_status_text(status));
      else
        (void)snprintf(message, sizeof message, "Copy was not created: %s.",
                       umi_status_text(status));
      runtime_show_notification(runtime, message);
    }
  }
  g_clear_object(&window);
  return status;
}

static void StudioSaveCopyComplete(GObject *source, GAsyncResult *result, gpointer data) {
  StudioSaveCopyRequest *request = data;
  GError *error = NULL;
  GFile *file = gtk_file_dialog_save_finish(GTK_FILE_DIALOG(source), result, &error);
  if (file != NULL) {
    char *path = g_file_get_path(file);
    if (path != NULL)
      (void)StudioSaveCopyWrite(request, path);
    else {
      GObject *window = NULL;
      UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request->owner, &window);
      if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window)))
        runtime_show_notification(runtime, "Save Copy requires a local file destination.");
      g_clear_object(&window);
    }
    g_free(path);
  } else if (error != NULL && !runtime_file_dialog_cancelled(error)) {
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request->owner, &window);
    if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window)))
      runtime_show_notification(runtime, error->message);
    g_clear_object(&window);
  }
  g_clear_object(&file);
  g_clear_error(&error);
  StudioSaveCopyFree(request);
}

static UmiStatus RuntimeSaveCopy(UmiStudioGtkRuntimeChrome *runtime, const char *destination) {
  if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL)
    return UMI_STATUS_INVALID_ARGUMENT;
  UmiDocumentCoordinator *documents = umi_studio_ui_documents(runtime->owner->ui);
  UmiDocumentWorkingCopySnapshot active;
  UmiStatus status = umi_document_coordinator_active_snapshot(documents, &active);
  if (status != UMI_STATUS_OK)
    return status;
  StudioSaveCopyRequest *request = g_try_new0(StudioSaveCopyRequest, 1);
  if (request == NULL)
    return UMI_STATUS_OUT_OF_MEMORY;
  request->owner = runtime_file_dialog_request_new(runtime, active.document_id);
  if (request->owner == NULL) {
    StudioSaveCopyFree(request);
    return UMI_STATUS_OUT_OF_MEMORY;
  }
  /* Capture before constructing a native dialog: notification handlers can
   * switch tabs or edit text. The selection still refers to this exact draft. */
  status = UmiDocumentCoordinatorCaptureCopy(documents, active.document_id, &request->copy);
  if (status != UMI_STATUS_OK) {
    StudioSaveCopyFree(request);
    return status;
  }
  if (destination != NULL && destination[0] != '\0') {
    status = StudioSaveCopyWrite(request, destination);
    StudioSaveCopyFree(request);
    return status;
  }
  UmiStudioWorkspaceSnapshot workspace;
  bool haveWorkspace = umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui),
                                                     &workspace) == UMI_STATUS_OK;
  char parent[UMI_PATH_CAPACITY] = {0};
  const char *folder = haveWorkspace && workspace.graph.open ? workspace.graph.root : NULL;
  if (active.has_path && umi_path_parent(active.path, parent, sizeof parent) == UMI_STATUS_OK)
    folder = parent;
  char name[UMI_DOCUMENT_NAME_CAPACITY + 8U];
  (void)snprintf(name, sizeof name, "copy-%s", active.display_name);
  GtkFileDialog *dialog = gtk_file_dialog_new();
  GObject *window = NULL;
  runtime = runtime_file_dialog_owner(request->owner, &window);
  status = UMI_STATUS_CANCELLED;
  if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window))) {
    gtk_file_dialog_set_title(dialog, "Save Copy of Captured Draft");
    if (UmiPathPickerGtk4SetInitialLocation(dialog, UMI_PATH_PICKER_NEW_FILE, NULL, folder, name) !=
        UMI_STATUS_OK)
      (void)UmiPathPickerGtk4SetInitialLocation(dialog, UMI_PATH_PICKER_NEW_FILE, NULL, NULL, NULL);
    runtime = RuntimeResolveOwner(request->owner, &window);
    if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window))) {
      gtk_file_dialog_save(dialog, GTK_WINDOW(window), NULL, StudioSaveCopyComplete, request);
      request = NULL;
      status = UMI_STATUS_OK;
    }
  }
  g_clear_object(&window);
  g_object_unref(dialog);
  StudioSaveCopyFree(request);
  return status;
}
