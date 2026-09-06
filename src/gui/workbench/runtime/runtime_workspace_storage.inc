/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/runtime_workspace_storage.inc
 * Purpose: Present explicit Framework canvas checkpoints and safe recovery.
 * Author: Sammy Hegab, Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

/* Layout preference belongs to this application, not to an open source file.
 * The user configuration directory separates operating-system user accounts. */
static const UmiUiWorkspaceCheckpointScope STUDIO_CANVAS_SCOPE = {
    "org.umicom.studio", "desktop", UMI_STUDIO_RUNTIME_WORKSPACE_PREFIX
};

/* Describe a stored checkpoint without claiming the current arrangement has
 * no newer edits. Unsupported storage never looks like a successful disk save. */
static void runtime_workspace_update_storage_controls(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    const char *label = "Storage unavailable";
    const char *detail = "No canvas Data Server is connected";
    int ready;
    if (runtime == NULL) return;
    ready = runtime->workspace_storage != NULL && model != NULL && !model->edit_active;
    if (runtime->workspace_storage != NULL) {
        if (!runtime->workspace_checkpoint.storage_revision_known) {
            label = "Storage needs attention";
            detail = "Restore can retry recovery; Save needs trustworthy storage metadata";
        } else if (runtime->workspace_storage_status != UMI_STATUS_OK &&
                   runtime->workspace_storage_status != UMI_STATUS_NOT_FOUND) {
            label = "Checkpoint error";
            detail = umi_status_text(runtime->workspace_storage_status);
        } else if (runtime->workspace_checkpoint.recovered_last_good) {
            label = "Previous checkpoint";
            detail = "The latest checkpoint was unusable; a validated previous copy was recovered";
        } else if (runtime->workspace_checkpoint.layout_id[0] != '\0') {
            label = runtime->workspace_checkpoint.durable ? "Checkpoint on disk" : "Memory checkpoint";
            detail = "Only the last explicitly saved layout is stored; newer arrangements still need Save layout";
        } else {
            label = "No saved checkpoint";
            detail = "Apply and Lock, then Save layout to store the current arrangement";
        }
    }
    if (runtime->workspace_save_button != NULL)
        gtk_widget_set_sensitive(runtime->workspace_save_button,
            ready && runtime->workspace_checkpoint.storage_revision_known);
    if (runtime->workspace_restore_button != NULL)
        gtk_widget_set_sensitive(runtime->workspace_restore_button, ready);
    if (runtime->workspace_storage_label != NULL) {
        gtk_label_set_text(GTK_LABEL(runtime->workspace_storage_label), label);
        gtk_widget_set_tooltip_text(runtime->workspace_storage_label, detail);
    }
}

/* Decode through the existing importer on a heap candidate. No live model is
 * replaced until the shared native host accepts its complete presentation. */
static UmiStatus runtime_workspace_publish_checkpoint(
    UmiStudioGtkRuntimeChrome *runtime, const char *text)
{
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    UmiUiWorkspaceCustomisation *candidate = g_try_new(UmiUiWorkspaceCustomisation, 1);
    UmiUiWorkspaceImportOptions options = umi_ui_workspace_import_options_default();
    UmiStatus status;
    if (candidate == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    *candidate = *model;
    options.conflict_policy = UMI_UI_WORKSPACE_IMPORT_REPLACE_CONFLICT;
    options.activate_imported_layout = true;
    options.allow_new_layout = true;
    status = umi_ui_workspace_customisation_import(candidate, text, &options, NULL);
    if (status == UMI_STATUS_OK)
        status = umi_gtk4_workspace_layout_host_rebuild(runtime->workspace_host,
            umi_ui_workspace_customisation_active_const(candidate));
    if (status == UMI_STATUS_OK) {
        *model = *candidate;
        (void)umi_gtk4_workspace_layout_host_set_context_groups(runtime->workspace_host, &model->groups);
        (void)umi_studio_runtime_workspace_project_shell(model, runtime->shell_registry,
            &runtime->shell_state, &runtime->shell_layout);
        runtime_workspace_update_cache(runtime);
    }
    g_free(candidate);
    return status;
}

/* The storage revision is captured during binding or restore, never silently
 * refreshed before Save. Another process's checkpoint therefore causes a
 * conflict instead of being overwritten by an older Studio window. */
static UmiStatus runtime_workspace_save(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    UmiUiWorkspaceCheckpointReport report;
    UmiStatus status;
    if (runtime == NULL || model == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (runtime->workspace_storage == NULL) status = UMI_STATUS_UNAVAILABLE;
    else if (model->edit_active) status = UMI_STATUS_BUSY;
    else if (!runtime->workspace_checkpoint.storage_revision_known) status = UMI_STATUS_INVALID_STATE;
    else {
        const gint64 now = g_get_real_time();
        /* A timestamp is descriptive, not a revision. Refuse arithmetic wrap
         * even if a platform clock supplies an unusually large future date. */
        const uint64_t saved_at_ns = now > 0 && (uint64_t)now <= UINT64_MAX / UINT64_C(1000)
            ? (uint64_t)now * UINT64_C(1000) : 0U;
        status = umi_ui_workspace_checkpoint_save(runtime->workspace_storage,
            &STUDIO_CANVAS_SCOPE, model, saved_at_ns,
            runtime->workspace_checkpoint.storage_revision, &report);
        if (status == UMI_STATUS_OK) runtime->workspace_checkpoint = report;
        /* Do not adopt a competing writer's revision after a failed Save.
         * Unreadable metadata also suspends saving until an explicit reload. */
        else if (!report.storage_revision_known && report.primary_status != UMI_STATUS_NOT_FOUND)
            runtime->workspace_checkpoint.storage_revision_known = false;
    }
    runtime->workspace_storage_status = status;
    runtime_workspace_update_storage_controls(runtime);
    runtime_show_notification(runtime, status == UMI_STATUS_OK
        ? (runtime->workspace_checkpoint.durable
            ? "Layout checkpoint saved on disk. Source files are saved separately."
            : "Layout checkpoint saved in memory only; it will not survive restart.")
        : (status == UMI_STATUS_BUSY ? "Apply or cancel the layout edit before saving."
            : "Layout was not saved. Check storage status; restore before retrying a revision conflict."));
    return status;
}

/* Loading checks product scope, catalogue, geometry and integrity in Framework.
 * A failed read or rejected candidate leaves the visible arrangement intact. */
static UmiStatus runtime_workspace_restore(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    UmiUiWorkspaceCheckpointReport report;
    char *text;
    UmiStatus status;
    if (runtime == NULL || model == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (runtime->workspace_storage == NULL) return UMI_STATUS_UNAVAILABLE;
    if (model->edit_active) {
        runtime_show_notification(runtime, "Apply or cancel the layout edit before restoring.");
        return UMI_STATUS_BUSY;
    }
    text = g_try_malloc(UMI_UI_LAYOUT_ENCODED_CAPACITY);
    if (text == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_ui_workspace_checkpoint_load_validated(runtime->workspace_storage,
        &STUDIO_CANVAS_SCOPE, model, text, UMI_UI_LAYOUT_ENCODED_CAPACITY, &report);
    if (status == UMI_STATUS_OK) status = runtime_workspace_publish_checkpoint(runtime, text);
    /* Publish revision evidence together with the accepted arrangement. A
     * busy connection or failed native rebuild must not authorise overwriting
     * a newer checkpoint, or erase a previously known revision before reading. */
    if (status == UMI_STATUS_OK || status == UMI_STATUS_NOT_FOUND)
        runtime->workspace_checkpoint = report;
    else if (!report.storage_revision_known && report.primary_status != UMI_STATUS_NOT_FOUND)
        runtime->workspace_checkpoint.storage_revision_known = false;
    g_free(text);
    runtime->workspace_storage_status = status;
    runtime_workspace_update_storage_controls(runtime);
    runtime_show_notification(runtime, status == UMI_STATUS_OK
        ? (report.recovered_last_good
            ? "Recovered the previous valid layout checkpoint. The unusable latest record was preserved."
            : "Restored the saved layout. Open documents and their drafts remain in memory.")
        : (status == UMI_STATUS_NOT_FOUND ? "There is no saved canvas checkpoint yet."
            : "The checkpoint could not be restored. The current layout was kept."));
    return status;
}

/* Explicit binding lets native acceptance tools use an isolated Data Server.
 * Read errors are reported without creating a different backend or deleting
 * damaged records. The caller keeps a supplied connection alive until teardown. */
static UmiStatus runtime_workspace_bind_storage(UmiStudioGtkRuntimeChrome *runtime,
    UmiDataServer *server, int restore_saved)
{
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    char *text;
    UmiStatus status;
    if (runtime == NULL || model == NULL || server == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (model->edit_active) return UMI_STATUS_BUSY;
    text = g_try_malloc(UMI_UI_LAYOUT_ENCODED_CAPACITY);
    if (text == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    if (runtime->workspace_storage != server) {
        if (runtime->owns_workspace_storage) umi_data_server_destroy(runtime->workspace_storage);
        runtime->workspace_storage = server;
        runtime->owns_workspace_storage = 0;
    }
    status = umi_ui_workspace_checkpoint_load_validated(server, &STUDIO_CANVAS_SCOPE,
        model, text, UMI_UI_LAYOUT_ENCODED_CAPACITY, &runtime->workspace_checkpoint);
    if (status == UMI_STATUS_OK && restore_saved)
        status = runtime_workspace_publish_checkpoint(runtime, text);
    g_free(text);
    runtime->workspace_storage_status = status;
    runtime_workspace_update_storage_controls(runtime);
    /* An empty database is usable: its first Save compares against revision 0. */
    return status == UMI_STATUS_NOT_FOUND ? UMI_STATUS_OK : status;
}

/* Normal native startup opts into a per-user Framework SQLite connection.
 * Existing configured SQLite services are reused. Offline constructors with
 * persist_session zero never open a user directory or a canvas database. */
static void runtime_workspace_prepare_storage(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiDataServer *server;
    int owned = 0;
    UmiStatus status;
    runtime->workspace_storage_status = UMI_STATUS_UNAVAILABLE;
    if (!runtime->owner->options.persist_session) return;
    server = umi_studio_services_data_server(umi_studio_ui_services(runtime->owner->ui));
    if (server == NULL || umi_data_server_backend(server) != UMI_DATA_BACKEND_SQLITE) {
        status = umi_gtk4_workspace_storage_open("org.umicom.studio", &server);
        if (status != UMI_STATUS_OK) {
            runtime->workspace_storage_status = status;
            runtime_show_notification(runtime, "Canvas storage is unavailable; the default layout remains usable.");
            return;
        }
        owned = 1;
    }
    status = runtime_workspace_bind_storage(runtime, server, runtime->owner->options.restore_session);
    if (runtime->workspace_storage == server) runtime->owns_workspace_storage = owned;
    else if (owned) umi_data_server_destroy(server);
    if (status != UMI_STATUS_OK)
        runtime_show_notification(runtime, "Saved canvas could not be loaded; the default layout was kept.");
    else if (runtime->workspace_checkpoint.recovered_last_good)
        runtime_show_notification(runtime, "Recovered the previous valid canvas checkpoint; the latest record needs attention.");
}
