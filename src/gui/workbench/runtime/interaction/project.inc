/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/project.inc
 * PURPOSE: Connect Framework developer dialogs to the current Studio workspace.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

/* Adopt generated metadata only after the existing workspace path succeeds. */
static UmiStatus RuntimeProjectCreated(const UmiDeveloperProjectModel *model,
    const UmiBuildProfile *profile, void *context)
{
    UmiStudioGtkRuntimeChrome *runtime = context;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    char mainPath[UMI_PATH_CAPACITY];
    char viewId[UMI_UI_ID_CAPACITY];
    UmiStatus status = runtime_open_workspace_path(runtime, model->root);
    if (status != UMI_STATUS_OK) return status;
    status = UmiStudioBuildProfileSave(services, profile);
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime,
            "The project files were created, but its build settings could not be saved. Keep the project folder and check settings storage.");
        return status;
    }
    if (umi_fs_join(mainPath, sizeof(mainPath), model->root, "src/main.c") == UMI_STATUS_OK &&
        umi_fs_is_file(mainPath)) {
        status = umi_document_coordinator_open(umi_studio_ui_documents(runtime->owner->ui),
            mainPath, viewId, sizeof(viewId));
        if (status == UMI_STATUS_OK)
            status = umi_ui_workbench_activate_document(
                umi_studio_ui_workbench(runtime->owner->ui), viewId);
    }
    runtime_show_notification(runtime,
        "Project created. Edit and save your source, then review Build > Project Settings before building.");
    (void)runtime_refresh_services(runtime);
    return status;
}

/* The displayed source folder is immutable. Reject a stale dialog after any
 * workspace change instead of granting trust to a different folder. */
static UmiStatus RuntimeBuildSettingsApplied(const UmiBuildProfile *profile,
    int trusted, void *context)
{
    UmiStudioGtkRuntimeChrome *runtime = context;
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    UmiStatus status = umi_studio_workspace_snapshot(services, &workspace);
    if (status != UMI_STATUS_OK) return status;
    if (!workspace.graph.open || !umi_path_equal(workspace.graph.root, profile->source_directory))
        return UMI_STATUS_INVALID_STATE;
    if (UmiStudioBuildBusy(umi_studio_services_build(services))) return UMI_STATUS_BUSY;
    /* A rejected/stale settings write must not grant execution trust. Persist
     * the reviewed profile first; trust remains a separate explicit decision. */
    status = UmiStudioBuildProfileSave(services, profile);
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, status == UMI_STATUS_INVALID_STATE
            ? "Project settings changed elsewhere. Reopen the workspace to read the latest settings before applying changes."
            : "Project settings could not be saved. The previous profile and workspace trust were kept.");
        return status;
    }
    status = umi_studio_security_set_workspace_trust(services, workspace.graph.root,
        trusted ? UMI_WORKSPACE_TRUSTED : UMI_WORKSPACE_UNTRUSTED);
    if (status == UMI_STATUS_OK)
        status = umi_studio_workspace_set_trusted(services, trusted);
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime,
            "Project settings were saved, but workspace trust could not be changed. No build has been started.");
        return status;
    }
    if (status == UMI_STATUS_OK) runtime_show_notification(runtime,
        runtime->workspace_storage != NULL &&
        umi_data_server_backend(runtime->workspace_storage) == UMI_DATA_BACKEND_SQLITE
            ? "Project settings saved. Workspace trust must be reviewed again after reopening."
            : "Project settings applied in memory. Persistent user storage is not bound.");
    (void)runtime_refresh_services(runtime);
    return status;
}

/* Reuse one Framework form per owner and dispose its previous callbacks first. */
static UmiStatus RuntimeNewProject(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    if (UmiStudioBuildBusy(umi_studio_services_build(services))) return UMI_STATUS_BUSY;
    UmiGtk4DeveloperDialogDestroy(runtime->project_dialog);
    runtime->project_dialog = NULL;
    return UmiGtk4NewProjectDialogCreate(runtime->owner->window,
        RuntimeProjectCreated, runtime, &runtime->project_dialog);
}

/* Settings edit a copy. No compiler or child process starts when the form opens. */
static UmiStatus RuntimeBuildSettings(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    UmiStatus status = umi_studio_workspace_snapshot(services, &workspace);
    if (status != UMI_STATUS_OK) return status;
    if (!workspace.graph.open) return UMI_STATUS_INVALID_STATE;
    if (UmiStudioBuildBusy(umi_studio_services_build(services))) return UMI_STATUS_BUSY;
    UmiGtk4DeveloperDialogDestroy(runtime->build_settings_dialog);
    runtime->build_settings_dialog = NULL;
    return UmiGtk4BuildSettingsDialogCreate(runtime->owner->window,
        umi_studio_build_service_profile(umi_studio_services_build(services)),
        workspace.graph.trusted, RuntimeBuildSettingsApplied, runtime,
        &runtime->build_settings_dialog);
}

/* Save through the document coordinator before CMake reads source from disk.
 * Untitled documents require Save As first; do not pretend they reached disk. */
static UmiStatus RuntimeBackgroundBuild(UmiStudioGtkRuntimeChrome *runtime,
    const char *command)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    /* A pending filename decision belongs to the existing Save All session.
     * Do not start a second save path or a build over partially saved sources. */
    if (UmiGtk4AdapterDocumentSaveAllBusy(runtime->owner->adapter)) {
        runtime_show_notification(runtime,
            "Build did not start. Finish or cancel Save All first.");
        return UMI_STATUS_BUSY;
    }
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(runtime->owner->ui);
    UmiStatus status = UmiDocumentCoordinatorSaveAll(documents, NULL);
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime,
            "Build did not start. Save unnamed documents with Save As and resolve any file conflict first.");
        return status;
    }
    return runtime_execute_registered_command(runtime, command, "background");
}
