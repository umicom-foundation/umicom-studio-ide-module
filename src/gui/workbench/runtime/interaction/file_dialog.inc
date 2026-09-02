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

/*
 * Provide the runtime open folder complete operation used by this module and its client
 * applications.
 */
static void runtime_open_folder_complete(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStudioServices *services;
    UmiFileIndex *index;
    GError *error = NULL;
    GFile *folder;
    char *path;
    char *name;
    UmiStatus status;

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

    services = umi_studio_ui_services(runtime->owner->ui);

    /*
     * Selecting a folder in this explicit Open Folder command is treated as the
     * user's trust decision for this local workspace. Framework still enforces
     * command permissions and trust requirements after the workspace opens.
     */
    status = umi_studio_services_open_workspace(services, path, 1);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        index = umi_studio_services_file_index(services);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (index != NULL) {
            (void)umi_file_index_set_root(index, path);
            (void)umi_file_index_rebuild(index);
        }

        name = g_path_get_basename(path);
        /* Apply this branch only when its contract condition is satisfied. */
        if (strlen(path) < sizeof(runtime->workspace_root)) {
            (void)strcpy(runtime->workspace_root, path);
        }
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (name != NULL &&
            strlen(name) < sizeof(runtime->workspace_name)) {
            (void)strcpy(runtime->workspace_name, name);
        }
        g_free(name);

        runtime_show_notification(runtime, "Workspace opened.");
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        runtime_show_notification(runtime, umi_status_text(status));
    }

    g_free(path);
    g_object_unref(folder);
    (void)runtime_refresh_services(runtime);
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
