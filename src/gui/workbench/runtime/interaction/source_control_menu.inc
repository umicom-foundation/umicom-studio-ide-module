/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/source_control_menu.inc
 *
 * PURPOSE:
 *   Reveal the existing branch list through Framework notebook ownership without issuing repository operations.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Availability and execution use the same workspace boundary. An unavailable
 * Git provider still has a useful empty/unavailable panel; viewing it does not
 * grant permission to run a repository operation. */
static int RuntimeBranchesCommandEnabled(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioWorkspaceSnapshot workspace;
    return runtime != NULL && runtime->owner != NULL && runtime->owner->ui != NULL &&
        UmiGtk4WindowIsOpen(runtime->owner->window) &&
        g_object_get_data(G_OBJECT(runtime->owner->window), "umicom-branches-menu-active") == NULL &&
        umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui),
            &workspace) == UMI_STATUS_OK && workspace.graph.open;
}

static UmiStatus RuntimeRevealBranches(UmiStudioGtkRuntimeChrome *runtime,
    const char *argument)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL ||
        (argument != NULL && argument[0] != '\0')) return UMI_STATUS_INVALID_ARGUMENT;
    if (!UmiGtk4WindowIsOpen(runtime->owner->window)) return UMI_STATUS_CANCELLED;
    if (g_object_get_data(G_OBJECT(runtime->owner->window), "umicom-branches-menu-active") != NULL)
        return UMI_STATUS_BUSY;
    UmiStudioWorkspaceSnapshot before;
    UmiStatus status = umi_studio_workspace_snapshot(
        umi_studio_ui_services(runtime->owner->ui), &before);
    if (status != UMI_STATUS_OK) return status;
    if (!before.graph.open) return UMI_STATUS_INVALID_STATE;

    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GObject *guard = g_object_ref(G_OBJECT(runtime->owner->window)), *window = NULL;
    g_object_set_data(guard, "umicom-branches-menu-active", GINT_TO_POINTER(1));
    status = runtime_show_surface(runtime, UMI_STUDIO_SURFACE_SOURCE_CONTROL);
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) status = UMI_STATUS_CANCELLED;
    UmiStudioWorkspaceSnapshot after;
    if (status == UMI_STATUS_OK &&
        (umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui),
             &after) != UMI_STATUS_OK || !after.graph.open ||
         after.graph.revision != before.graph.revision ||
         strcmp(after.graph.root, before.graph.root) != 0)) status = UMI_STATUS_CANCELLED;
    if (status == UMI_STATUS_OK) {
        GtkWidget *target = runtime->source_control_branches_list;
        GtkWidget *notebook = target != NULL
            ? gtk_widget_get_ancestor(target, GTK_TYPE_NOTEBOOK) : NULL;
        status = GTK_IS_LIST_BOX(target) && GTK_IS_NOTEBOOK(notebook)
            ? UmiGtk4NotebookSelectOwnedTarget(GTK_WINDOW(window), GTK_NOTEBOOK(notebook), target)
            : UMI_STATUS_UNAVAILABLE;
        /* Page observers may retire the host or switch its project. No borrowed
         * runtime or service pointer crosses the Framework presentation call. */
        g_clear_object(&window);
        runtime = runtime_file_dialog_owner(request, &window);
        if (runtime == NULL) status = UMI_STATUS_CANCELLED;
        if (status == UMI_STATUS_OK &&
            (umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui),
                 &after) != UMI_STATUS_OK || !after.graph.open ||
             after.graph.revision != before.graph.revision ||
             strcmp(after.graph.root, before.graph.root) != 0)) status = UMI_STATUS_CANCELLED;
    }
    g_object_set_data(guard, "umicom-branches-menu-active", NULL);
    g_object_unref(guard);
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}
