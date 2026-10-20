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
#include "umicom/ui/gtk4/path_picker.h"

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
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->window == NULL) return NULL;
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

/* Opening a source is an explicit request to edit. Reveal the retained editor
 * even when a saved layout hid it; periodic refresh never makes this decision.
 * Resolve the lifetime token after operations which can notify native observers. */
static UmiStatus RuntimePresentActiveSource(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GObject *window = NULL;
    UmiStatus status = runtime_workspace_open_surface(runtime, UMI_STUDIO_SURFACE_EDITOR, NULL);
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) status = UMI_STATUS_CANCELLED;
    else if (status == UMI_STATUS_OK) {
        /* Status panels are independent of document availability. Refresh them
         * normally, then ask Framework to render and focus the exact document. */
        (void)runtime_refresh_services(runtime);
        g_clear_object(&window);
        runtime = runtime_file_dialog_owner(request, &window);
        if (runtime == NULL) status = UMI_STATUS_CANCELLED;
        else {
            runtime_welcome_update_mode(runtime);
            g_clear_object(&window);
            runtime = runtime_file_dialog_owner(request, &window);
            status = runtime != NULL ? UmiGtk4AdapterFocusActiveDocument(runtime->owner->adapter)
                                     : UMI_STATUS_CANCELLED;
        }
    }
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}

/* Choosers, Explorer and commands share the same Framework document owner.
 * A path argument is a local source file; it does not grant project trust. */
static UmiStatus RuntimeOpenSourcePath(UmiStudioGtkRuntimeChrome *runtime, const char *path)
{
    if (runtime == NULL || runtime->owner == NULL || path == NULL || path[0] == '\0')
        return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    /* Explorer rows and command arguments can be released by document-change
     * observers. Keep the selected path independently until opening finishes. */
    char *selectedPath = g_strdup(path);
    char viewId[UMI_UI_ID_CAPACITY];
    UmiStatus status = umi_document_coordinator_open(
        umi_studio_ui_documents(runtime->owner->ui), selectedPath, viewId, sizeof viewId);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) status = UMI_STATUS_CANCELLED;
    else if (status == UMI_STATUS_OK) {
        runtime_copy_selection(runtime->selected_file_path, sizeof runtime->selected_file_path, selectedPath);
        status = RuntimePresentActiveSource(runtime);
    }
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    g_free(selectedPath);
    return status;
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
    /* The shared opening route owns its temporary view identity. */
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

/* Opening now reveals and focuses the real editor through the shared Studio route; retain the earlier model-only opening sequence for review. */
#if 0
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
#endif
    status = RuntimeOpenSourcePath(runtime, path);
    g_clear_object(&ownerWindow);
    runtime = runtime_file_dialog_owner(request, &ownerWindow);
    if (runtime != NULL && status != UMI_STATUS_OK)
        runtime_show_notification(runtime, umi_status_text(status));

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
/* Save completion now rechecks the retained window token before refreshing or showing a result. The earlier raw-pointer route is retained for comparison. */
#if 0
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
#endif
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
    /* Saving notifies document observers. Reacquire the host both after the
     * write and after refresh, so a closed window receives no late feedback. */
    g_clear_object(&ownerWindow);
    runtime = runtime_file_dialog_owner(request, &ownerWindow);
    if (runtime == NULL) goto cleanup;
    (void)runtime_refresh_services(runtime);
    g_clear_object(&ownerWindow);
    runtime = runtime_file_dialog_owner(request, &ownerWindow);
    if (runtime == NULL) goto cleanup;
    if (status == UMI_STATUS_OK) {
        runtime_show_notification(runtime, "Document saved.");
    } else {
        char message[384];
        (void)snprintf(message, sizeof message,
            "Save As failed: %s. Review the destination and try again.", umi_status_text(status));
        runtime_show_notification(runtime, message);
    }

cleanup:
    g_free(path);
    g_clear_object(&file);
    g_clear_error(&error);
    g_clear_object(&ownerWindow);
    runtime_file_dialog_request_free(request);
}

/* Open a known workspace path without displaying a second chooser. Welcome
 * page recent-work buttons and the folder dialog both use this one route. */
/* Opening a large project must not recursively enumerate every source on the GTK thread. Retain the prior synchronous route; the replacement prepares the same services and starts Framework background indexing. */
#if 0
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

#endif
static UmiStatus runtime_open_workspace_path(UmiStudioGtkRuntimeChrome *runtime, const char *path)
{
    if (runtime == NULL || runtime->owner == NULL || path == NULL || path[0] == '\0')
        return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    char *selectedPath = g_strdup(path);
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStatus indexStatus = UMI_STATUS_UNAVAILABLE;
    UmiStatus status = UmiStudioWorkspaceOpenForEditing(services, selectedPath, 0, 1, &indexStatus);
    UmiStatus sessionStatus = UMI_STATUS_OK;
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) status = UMI_STATUS_CANCELLED;
    else if (status == UMI_STATUS_OK) {
        UmiStudioWorkspaceSnapshot workspace;
        char workspaceName[UMI_PATH_CAPACITY] = {0};
        const char *openedRoot = selectedPath;
        if (umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK)
            openedRoot = workspace.graph.root;
        /* A path selected in the previous repository is not a selection in
         * this project, even when both repositories contain the same filename. */
        runtime->selected_vcs_path[0] = '\0';
        runtime_copy_selection(runtime->workspace_root, sizeof runtime->workspace_root, openedRoot);
        if (umi_path_basename(openedRoot, workspaceName, sizeof workspaceName) == UMI_STATUS_OK)
            runtime_copy_selection(runtime->workspace_name, sizeof runtime->workspace_name, workspaceName);
        /* Session history is optional. It does not redefine project success or
         * wait for the recursive scan to complete. */
        sessionStatus = umi_studio_session_set_workspace(services, openedRoot);
        if (sessionStatus == UMI_STATUS_OK) sessionStatus = umi_studio_session_save(services);
    }
    g_clear_object(&window);
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL) (void)runtime_refresh_services(runtime);
    g_clear_object(&window);
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL) {
        char message[384];
        if (status == UMI_STATUS_OK) {
            (void)snprintf(message, sizeof message, "%s%s",
                indexStatus == UMI_STATUS_OK
                    ? "Workspace opened. File indexing runs in the background; see Explorer for progress."
                    : "Workspace opened. File indexing could not start; use Refresh files in Explorer to retry.",
                sessionStatus == UMI_STATUS_OK ? "" : " The workspace could not be saved for the next start.");
        } else if (status == UMI_STATUS_BUSY) {
            (void)snprintf(message, sizeof message,
                "The current workspace still has an active task. Wait for it to finish, then open the project again.");
        } else {
/* Report whether a failed adoption left a project selected. The earlier status-only message is retained for review. */
#if 0
            (void)snprintf(message, sizeof message, "Open project: %s", umi_status_text(status));
#endif
            UmiStudioWorkspaceSnapshot after;
            int closed = umi_studio_workspace_snapshot(services, &after) == UMI_STATUS_OK && !after.graph.open;
            (void)snprintf(message, sizeof message, closed
                ? "Open project: %s. No project is active; your open documents are retained. Select a project folder to retry."
                : "Open project: %s. The previous project is still selected.", umi_status_text(status));
        }
        runtime_show_notification(runtime, message);
    }
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    g_free(selectedPath);
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
/* Choose an initial location from the active document or selected project.
 * Settings storage and the launch directory are never used as project paths. */
static void RuntimePrepareDocumentChooser(UmiStudioGtkRuntimeChrome *runtime,
    GtkFileDialog *dialog, UmiPathPickerMode mode,
    const UmiDocumentWorkingCopySnapshot *active)
{
    UmiStudioWorkspaceSnapshot workspace;
    char parent[UMI_PATH_CAPACITY] = {0};
    const char *folder = NULL, *selected = NULL, *name = NULL;
    if (umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui), &workspace) == UMI_STATUS_OK && workspace.graph.open)
        folder = workspace.graph.root;
    if (active != NULL && active->has_path && umi_path_is_absolute(active->path)) {
        if (mode == UMI_PATH_PICKER_NEW_FILE) selected = active->path;
        else if (mode == UMI_PATH_PICKER_EXISTING_FILE &&
            umi_path_parent(active->path, parent, sizeof parent) == UMI_STATUS_OK)
            folder = parent;
    }
    if (mode == UMI_PATH_PICKER_NEW_FILE && active != NULL) name = active->display_name;
    if (UmiPathPickerGtk4SetInitialLocation(dialog, mode, selected, folder, name) != UMI_STATUS_OK) {
        /* A rejected hint must not prevent the user choosing a valid location.
         * Clear it without guessing another path or creating any directory. */
        (void)UmiPathPickerGtk4SetInitialLocation(dialog, mode, NULL, NULL, NULL);
    }
}

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
    UmiDocumentWorkingCopySnapshot active;
    UmiStatus activeStatus = umi_document_coordinator_active_snapshot(
        umi_studio_ui_documents(runtime->owner->ui), &active);
    RuntimePrepareDocumentChooser(runtime, dialog, UMI_PATH_PICKER_EXISTING_FILE,
        activeStatus == UMI_STATUS_OK ? &active : NULL);
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
    RuntimePrepareDocumentChooser(runtime, dialog, UMI_PATH_PICKER_NEW_FILE, &active);
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
    RuntimePrepareDocumentChooser(runtime, dialog, UMI_PATH_PICKER_DIRECTORY, NULL);
    gtk_file_dialog_select_folder(
        dialog,
        runtime->owner->window,
        NULL,
        runtime_open_folder_complete,
        request);
    g_object_unref(dialog);
    return UMI_STATUS_OK;
}
