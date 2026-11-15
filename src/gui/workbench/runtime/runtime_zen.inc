/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/runtime_zen.inc
 *
 * PURPOSE:
 *   Publish the Framework Zen transition through the existing native workspace owner.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Keep rendering and return-state publication in one native command. Failed
 * candidate validation or rendering never commits a different return identity.
 * Layout selection uses established presets; no new panel arrangement is made. */
static UmiStatus RuntimeToggleZen(UmiStudioGtkRuntimeChrome *runtime, const char *argument)
{
    if (runtime == NULL || (argument != NULL && argument[0] != '\0'))
        return UMI_STATUS_INVALID_ARGUMENT;
    if (runtime->zen_toggle_active) return UMI_STATUS_BUSY;
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    if (model == NULL) return UMI_STATUS_INVALID_STATE;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    UmiUiWorkspaceCustomisation *candidate = g_try_new(UmiUiWorkspaceCustomisation, 1);
    if (candidate == NULL) {
        runtime_file_dialog_request_free(request);
        return UMI_STATUS_OUT_OF_MEMORY;
    }
    *candidate = *model;
    UmiStudioWorkspaceZenState next = runtime->zen_state;
    UmiStatus status = umi_studio_runtime_workspace_toggle_zen(candidate, &next);
    GObject *window = NULL;
    if (status == UMI_STATUS_OK) {
        runtime->zen_toggle_active = true;
        status = runtime_workspace_publish_candidate(runtime, candidate);
        runtime = runtime_file_dialog_owner(request, &window);
        if (runtime == NULL) status = UMI_STATUS_CANCELLED;
        else {
            if (status == UMI_STATUS_OK) runtime->zen_state = next;
            runtime->zen_toggle_active = false;
            /* The existing refresh projects accepted model state into chrome,
             * command availability and status. It does not persist a layout. */
            if (status == UMI_STATUS_OK) status = runtime_refresh_services(runtime);
        }
    }
    g_clear_object(&window);
    g_free(candidate);
    runtime_file_dialog_request_free(request);
    return status;
}
