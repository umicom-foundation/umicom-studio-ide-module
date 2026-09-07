/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_shell.inc
 *
 * PURPOSE:
 *   Compose a professional Studio shell around the established Framework GTK4
 *   editor. One Framework workspace host renders named docks and free canvas
 *   placements from the existing professional customisation model.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Compose Studio chrome around the original editor and one shared workspace
 * host. The centre stack is an editor body only; it is not another dock tree. */
static UmiStatus runtime_build_shell(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *existing;
    GtkWidget *workbench;
    GtkWidget *host_widget;
    UmiStatus status;

    if (runtime == NULL || runtime->owner == NULL || runtime->owner->window == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    /* Studio supplies outer chrome while Framework retains the editor,
     * document groups, services and native panel geometry implementation. */
    status = umi_gtk4_adapter_set_chrome_visibility(runtime->owner->adapter, 0U);
    if (status != UMI_STATUS_OK) return status;
    existing = gtk_window_get_child(runtime->owner->window);
    if (existing == NULL) return UMI_STATUS_INVALID_STATE;
    runtime->editor_adapter_root = g_object_ref(existing);
    (void)umi_gtk4_automation_tag_widget(existing, "studio.workspace.editor-root");
    gtk_window_set_child(runtime->owner->window, NULL);

    runtime->outer_root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    g_object_ref_sink(runtime->outer_root);
    runtime->application_bar = runtime_build_application_bar(runtime);
    runtime->toolbar = runtime_build_toolbar(runtime);
    runtime->activity_bar = runtime_build_activity_bar(runtime);
    runtime->body = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    runtime->centre_stack = gtk_stack_new();
    g_object_ref_sink(runtime->centre_stack);
    gtk_stack_set_hhomogeneous(GTK_STACK(runtime->centre_stack), FALSE);
    gtk_stack_set_vhomogeneous(GTK_STACK(runtime->centre_stack), FALSE);
    gtk_widget_set_hexpand(runtime->centre_stack, TRUE);
    gtk_widget_set_vexpand(runtime->centre_stack, TRUE);
    runtime->document_tabs = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    runtime->status_bar = runtime_build_status_bar(runtime);
    runtime->editor_command_bar = runtime_build_editor_command_bar(runtime);
    workbench = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    runtime->workbench_surface = workbench;
    gtk_box_append(GTK_BOX(workbench), runtime->document_tabs);
    if (runtime->editor_command_bar != NULL)
        gtk_box_append(GTK_BOX(workbench), runtime->editor_command_bar);
    gtk_widget_set_hexpand(existing, TRUE);
    gtk_widget_set_vexpand(existing, TRUE);
    gtk_box_append(GTK_BOX(workbench), existing);
    runtime->welcome_surface = runtime_build_welcome_surface(runtime);
    if (runtime->application_bar == NULL || runtime->toolbar == NULL ||
        runtime->activity_bar == NULL || runtime->status_bar == NULL ||
        runtime->welcome_surface == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    gtk_stack_add_titled(GTK_STACK(runtime->centre_stack),
        runtime->welcome_surface, "welcome", "Welcome");
    gtk_stack_add_titled(GTK_STACK(runtime->centre_stack),
        runtime->workbench_surface, "workbench", "Editor");
    /* Retain the editor body even in a genuinely blank canvas. Its documents
     * and native working copies must not depend on whether its panel is shown. */
    status = runtime_workspace_create(runtime);
    if (status != UMI_STATUS_OK) return status;
    /* Optional storage failure leaves the useful product default available.
     * This is never an automatic save of an unfinished arrangement. */
    runtime_workspace_prepare_storage(runtime);
    host_widget = umi_gtk4_workspace_layout_host_widget(runtime->workspace_host);
    gtk_widget_set_hexpand(host_widget, TRUE);
    gtk_widget_set_vexpand(host_widget, TRUE);
    gtk_widget_set_hexpand(runtime->body, TRUE);
    gtk_widget_set_vexpand(runtime->body, TRUE);
    gtk_box_append(GTK_BOX(runtime->body), runtime->activity_bar);
    /* Named Framework edge tabs now provide visible tool navigation. Keep
     * the old action widgets parent-owned for existing command bindings,
     * without displaying a second oversized icon rail beside the tools. */
    gtk_widget_set_visible(runtime->activity_bar, FALSE);
    gtk_box_append(GTK_BOX(runtime->body), host_widget);
    runtime->layout_tabs = runtime_build_layout_tabs(runtime);
    if (runtime->layout_tabs == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->application_bar);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->toolbar);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->body);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->layout_tabs);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->status_bar);
    runtime_workspace_update_cache(runtime);

    /* A moderate first-run size fits common laptop displays. Canvas geometry
     * is normalized inside the actual host allocation, not this preferred size. */
    (void)umi_gtk4_ws_window_fit(runtime->owner->window, 1180, 760, 720, 480);
    gtk_window_set_title(runtime->owner->window, "Umicom Studio IDE");
    (void)umi_gtk4_ws_apply_window_identity(runtime->owner->window);
    gtk_window_set_child(runtime->owner->window, runtime->outer_root);
    runtime_welcome_update_mode(runtime);
    return UMI_STATUS_OK;
}
