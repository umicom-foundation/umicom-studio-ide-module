/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/file_dialog.inc
 *
 * PURPOSE:
 *   Collect file/folder paths with GTK4 and then hand those paths to existing
 *   Framework document/workspace services. File IO itself is not implemented
 *   here.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/session.h"

/* Native requests can complete after a tab change or after Studio closes.
 * The window stores a non-owning runtime binding which teardown clears. */
typedef struct UmiStudioFileDialogRequest {
    GWeakRef window;
    GObject *binding;
    UmiDocumentId documentId;
} UmiStudioFileDialogRequest;

static UmiStudioFileDialogRequest *runtime_file_dialog_request_new(
    UmiStudioGtkRuntimeChrome *runtime, UmiDocumentId documentId)
{
    UmiStudioFileDialogRequest *request = g_try_new0(UmiStudioFileDialogRequest, 1);
    if (request == NULL) return NULL;
    GObject *window = G_OBJECT(runtime->owner->window);
    GObject *binding = g_object_get_data(window, "umicom-studio-file-dialog-binding");
    if (binding == NULL || g_object_get_data(binding, "runtime") != runtime) {
        binding = g_object_new(G_TYPE_OBJECT, NULL);
        g_object_set_data(binding, "runtime", runtime);
        g_object_set_data_full(window, "umicom-studio-file-dialog-binding",
            binding, g_object_unref);
    }
    g_weak_ref_init(&request->window, window);
    request->binding = g_object_ref(binding);
    request->documentId = documentId;
    return request;
}

static UmiStudioGtkRuntimeChrome *runtime_file_dialog_owner(
    UmiStudioFileDialogRequest *request, GObject **outWindow)
{
    *outWindow = request != NULL ? g_weak_ref_get(&request->window) : NULL;
    /* A retained token also rejects callbacks from an earlier runtime if
     * the same window is later attached to a new owner. */
    if (*outWindow == NULL || g_object_get_data(*outWindow,
            "umicom-studio-file-dialog-binding") != request->binding) return NULL;
    return g_object_get_data(request->binding, "runtime");
}

static void runtime_file_dialog_request_free(UmiStudioFileDialogRequest *request)
{
    if (request == NULL) return;
    g_weak_ref_clear(&request->window);
    g_clear_object(&request->binding);
    g_free(request);
}

/* Dismissing a chooser is an ordinary user action, not an error notification. */
static int runtime_file_dialog_cancelled(const GError *error)
{
    return error != NULL && (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED) ||
        g_error_matches(error, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_DISMISSED) ||
        g_error_matches(error, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_CANCELLED));
}

static void runtime_open_file_complete(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
    UmiStudioFileDialogRequest *request = user_data;
    GObject *ownerWindow = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &ownerWindow);
    GError *error = NULL;
    GFile *file = NULL;
    char *path = NULL;
    char view_id[UMI_UI_ID_CAPACITY];
    UmiStatus status;

    file = gtk_file_dialog_open_finish(
        GTK_FILE_DIALOG(source_object),
        result,
        &error);
    if (runtime == NULL) goto cleanup;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (file == NULL) {
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (error != NULL &&
            !runtime_file_dialog_cancelled(error)) {
            runtime_show_notification(runtime, error->message);
        }
        goto cleanup;
    }

    path = g_file_get_path(file);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (path == NULL) {
        runtime_show_notification(
            runtime,
            "The selected file does not have a local filesystem path.");
        goto cleanup;
    }

    (void)memset(view_id, 0, sizeof(view_id));
    status = umi_document_coordinator_open(
        umi_studio_ui_documents(runtime->owner->ui),
        path,
        view_id,
        sizeof(view_id));

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        runtime_copy_selection(
            runtime->selected_file_path,
            sizeof(runtime->selected_file_path),
            path);
        status = umi_ui_workbench_activate_document(
            umi_studio_ui_workbench(runtime->owner->ui),
            view_id);
    }

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }

    (void)runtime_refresh_services(runtime);
cleanup:
    g_free(path);
    g_clear_object(&file);
    g_clear_error(&error);
    g_clear_object(&ownerWindow);
    runtime_file_dialog_request_free(request);
}

/*
 * Provide the runtime save as complete operation used by this module and its client
 * applications.
 */
static void runtime_save_as_complete(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
    UmiStudioFileDialogRequest *request = user_data;
    GObject *ownerWindow = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &ownerWindow);
    GError *error = NULL;
    GFile *file = NULL;
    char *path = NULL;
    UmiStatus status;

    file = gtk_file_dialog_save_finish(
        GTK_FILE_DIALOG(source_object),
        result,
        &error);
    if (runtime == NULL) goto cleanup;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (file == NULL) {
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (error != NULL &&
            !runtime_file_dialog_cancelled(error)) {
            runtime_show_notification(runtime, error->message);
        }
        goto cleanup;
    }

    path = g_file_get_path(file);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (path == NULL) {
        runtime_show_notification(
            runtime,
            "The selected destination does not have a local path.");
        goto cleanup;
    }

    status = UmiDocumentCoordinatorSaveAs(
        umi_studio_ui_documents(runtime->owner->ui),
        request->documentId, path);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        runtime_show_notification(runtime, "Document saved.");
    }

    (void)runtime_refresh_services(runtime);
cleanup:
    g_free(path);
    g_clear_object(&file);
    g_clear_error(&error);
    g_clear_object(&ownerWindow);
    runtime_file_dialog_request_free(request);
}

/* Open a known workspace path without displaying a second chooser. Welcome
 * page recent-work buttons and the folder dialog both use this one route. */
static UmiStatus runtime_open_workspace_path(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *path)
{
    UmiStudioServices *services;
    char workspace_name[UMI_PATH_CAPACITY] = {0};
    UmiStatus status;

    if (runtime == NULL || runtime->owner == NULL || path == NULL ||
        path[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    services = umi_studio_ui_services(runtime->owner->ui);
    /* Use the same transactional workspace route as menus, headless clients
     * and automation. A newly selected folder starts untrusted and receives a
     * watcher; trust can be granted later through the explicit trust workflow. */
    status = umi_studio_workspace_open(services, path, 0, 1);
    if (status == UMI_STATUS_OK) {
        UmiStatus sessionStatus;

        runtime_copy_selection(
            runtime->workspace_root,
            sizeof(runtime->workspace_root),
            path);
        if (umi_path_basename(path, workspace_name,
                              sizeof(workspace_name)) == UMI_STATUS_OK) {
            runtime_copy_selection(
                runtime->workspace_name,
                sizeof(runtime->workspace_name),
                workspace_name);
        }

        /* Preserve the established successful-open feedback. Session persistence
         * is an additional convenience and must not change whether opening the
         * selected workspace itself succeeded. */
        runtime_show_notification(runtime, "Workspace opened.");

        /* Remember the selected workspace only after it is fully open. The
         * project remains usable if session persistence is unavailable; Studio
         * reports that separately instead of changing the open result. */
        sessionStatus = umi_studio_session_set_workspace(services, path);
        if (sessionStatus == UMI_STATUS_OK) {
            sessionStatus = umi_studio_session_save(services);
        }
        if (sessionStatus != UMI_STATUS_OK) {
            runtime_show_notification(
                runtime,
                "Workspace opened, but Studio could not save it for the next start.");
        }
    } else {
        runtime_show_notification(runtime, umi_status_text(status));
    }
    (void)runtime_refresh_services(runtime);
    return status;
}

/* Complete the asynchronous folder chooser and pass its local path to the
 * same workspace service used by recent-work actions. */
static void runtime_open_folder_complete(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
    UmiStudioFileDialogRequest *request = user_data;
    GObject *ownerWindow = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &ownerWindow);
    GError *error = NULL;
    GFile *folder = NULL;
    char *path = NULL;

    folder = gtk_file_dialog_select_folder_finish(
        GTK_FILE_DIALOG(source_object),
        result,
        &error);
    if (runtime == NULL) goto cleanup;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (folder == NULL) {
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (error != NULL &&
            !runtime_file_dialog_cancelled(error)) {
            runtime_show_notification(runtime, error->message);
        }
        goto cleanup;
    }

    path = g_file_get_path(folder);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (path == NULL) {
        runtime_show_notification(
            runtime,
            "The selected workspace does not have a local path.");
        goto cleanup;
    }

    (void)runtime_open_workspace_path(runtime, path);

cleanup:
    g_free(path);
    g_clear_object(&folder);
    g_clear_error(&error);
    g_clear_object(&ownerWindow);
    runtime_file_dialog_request_free(request);
}

/*
 * Provide the runtime choose file to open operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_choose_file_to_open(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkFileDialog *dialog;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    dialog = gtk_file_dialog_new();
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (dialog == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(
        runtime, 0U);
    if (request == NULL) {
        g_object_unref(dialog);
        return UMI_STATUS_OUT_OF_MEMORY;
    }
    gtk_file_dialog_set_title(dialog, "Open File");
    gtk_file_dialog_open(
        dialog,
        runtime->owner->window,
        NULL,
        runtime_open_file_complete,
        request);
    g_object_unref(dialog);
    return UMI_STATUS_OK;
}

/*
 * Provide the runtime choose save as operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_choose_save_as(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkFileDialog *dialog;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    dialog = gtk_file_dialog_new();
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (dialog == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    UmiDocumentWorkingCopySnapshot active;
    UmiStatus status = umi_document_coordinator_active_snapshot(
        umi_studio_ui_documents(runtime->owner->ui), &active);
    if (status != UMI_STATUS_OK) {
        g_object_unref(dialog);
        return status;
    }
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(
        runtime, active.document_id);
    if (request == NULL) {
        g_object_unref(dialog);
        return UMI_STATUS_OUT_OF_MEMORY;
    }
    gtk_file_dialog_set_title(dialog, "Save As");
    gtk_file_dialog_save(
        dialog,
        runtime->owner->window,
        NULL,
        runtime_save_as_complete,
        request);
    g_object_unref(dialog);
    return UMI_STATUS_OK;
}

/*
 * Provide the runtime choose workspace folder operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_choose_workspace_folder(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkFileDialog *dialog;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    dialog = gtk_file_dialog_new();
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (dialog == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(
        runtime, 0U);
    if (request == NULL) {
        g_object_unref(dialog);
        return UMI_STATUS_OUT_OF_MEMORY;
    }
    gtk_file_dialog_set_title(dialog, "Open Workspace Folder");
    gtk_file_dialog_select_folder(
        dialog,
        runtime->owner->window,
        NULL,
        runtime_open_folder_complete,
        request);
    g_object_unref(dialog);
    return UMI_STATUS_OK;
}
