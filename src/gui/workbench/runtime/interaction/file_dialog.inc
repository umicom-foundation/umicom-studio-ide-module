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

static void runtime_open_file_complete(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    GError *error = NULL;
    GFile *file;
    char *path;
    char view_id[UMI_UI_ID_CAPACITY];
    UmiStatus status;

    file = gtk_file_dialog_open_finish(
        GTK_FILE_DIALOG(source_object),
        result,
        &error);
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
            !g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
            runtime_show_notification(runtime, error->message);
        }
        g_clear_error(&error);
        return;
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
        g_object_unref(file);
        return;
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

    g_free(path);
    g_object_unref(file);
    (void)runtime_refresh_services(runtime);
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
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    GError *error = NULL;
    GFile *file;
    char *path;
    UmiStatus status;

    file = gtk_file_dialog_save_finish(
        GTK_FILE_DIALOG(source_object),
        result,
        &error);
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
            !g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
            runtime_show_notification(runtime, error->message);
        }
        g_clear_error(&error);
        return;
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
        g_object_unref(file);
        return;
    }

    status = umi_document_coordinator_save_active_as(
        umi_studio_ui_documents(runtime->owner->ui),
        path);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        runtime_show_notification(runtime, "Document saved.");
    }

    g_free(path);
    g_object_unref(file);
    (void)runtime_refresh_services(runtime);
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
        runtime_show_notification(runtime, "Workspace opened.");
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
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    GError *error = NULL;
    GFile *folder;
    char *path;

    folder = gtk_file_dialog_select_folder_finish(
        GTK_FILE_DIALOG(source_object),
        result,
        &error);
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
            !g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
            runtime_show_notification(runtime, error->message);
        }
        g_clear_error(&error);
        return;
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
        g_object_unref(folder);
        return;
    }

    (void)runtime_open_workspace_path(runtime, path);

    g_free(path);
    g_object_unref(folder);
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

    gtk_file_dialog_set_title(dialog, "Open File");
    gtk_file_dialog_open(
        dialog,
        runtime->owner->window,
        NULL,
        runtime_open_file_complete,
        runtime);
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

    gtk_file_dialog_set_title(dialog, "Save As");
    gtk_file_dialog_save(
        dialog,
        runtime->owner->window,
        NULL,
        runtime_save_as_complete,
        runtime);
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

    gtk_file_dialog_set_title(dialog, "Open Workspace Folder");
    gtk_file_dialog_select_folder(
        dialog,
        runtime->owner->window,
        NULL,
        runtime_open_folder_complete,
        runtime);
    g_object_unref(dialog);
    return UMI_STATUS_OK;
}
