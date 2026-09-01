/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_shell.inc
 *
 * PURPOSE:
 *   Compose a professional Studio shell around the established Framework GTK4
 *   editor. Native splitters make tool regions resizable while Framework state
 *   remains responsible for persistence and named layouts.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_configure_splitter(GtkWidget *paned)
{
    if (paned == NULL) return;
    gtk_widget_add_css_class(paned, "umicom-runtime-split");
    gtk_paned_set_wide_handle(GTK_PANED(paned), FALSE);
    gtk_paned_set_resize_start_child(GTK_PANED(paned), TRUE);
    gtk_paned_set_resize_end_child(GTK_PANED(paned), TRUE);
    /* Allow both children to contract when the operating system gives Studio
     * less room than the saved layout used on a larger monitor. */
    gtk_paned_set_shrink_start_child(GTK_PANED(paned), TRUE);
    gtk_paned_set_shrink_end_child(GTK_PANED(paned), TRUE);
}

static UmiStatus runtime_build_shell(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *existing;
    GtkWidget *workbench;
    UmiStatus status;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Studio supplies the outer chrome. The Framework adapter keeps ownership
     * of the editor groups, documents and toolkit-neutral dock state. */
    status = umi_gtk4_adapter_set_chrome_visibility(
        runtime->owner->adapter, 0U);
    if (status != UMI_STATUS_OK) return status;

    existing = gtk_window_get_child(runtime->owner->window);
    if (existing == NULL) return UMI_STATUS_INVALID_STATE;
    g_object_ref(existing);
    gtk_window_set_child(runtime->owner->window, NULL);

    runtime->outer_root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    runtime->application_bar = runtime_build_application_bar(runtime);
    runtime->toolbar = runtime_build_toolbar(runtime);
    runtime->workspace_paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    runtime->body = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    runtime->primary_paned = runtime->body;
    runtime->secondary_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    runtime->primary_cluster = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    runtime->activity_bar = runtime_build_activity_bar(runtime);
    runtime->primary_revealer = gtk_revealer_new();
    runtime->primary_stack = gtk_stack_new();
    runtime->secondary_revealer = gtk_revealer_new();
    runtime->secondary_stack = gtk_stack_new();
    runtime->bottom_revealer = gtk_revealer_new();
    runtime->bottom_stack = gtk_stack_new();
    runtime->centre_stack = gtk_stack_new();
    runtime->document_tabs = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    runtime->layout_tabs = runtime_build_layout_tabs(runtime);
    runtime->status_bar = runtime_build_status_bar(runtime);

    if (runtime->outer_root == NULL ||
        runtime->application_bar == NULL ||
        runtime->toolbar == NULL ||
        runtime->workspace_paned == NULL ||
        runtime->body == NULL ||
        runtime->secondary_paned == NULL ||
        runtime->primary_cluster == NULL ||
        runtime->activity_bar == NULL ||
        runtime->primary_revealer == NULL ||
        runtime->primary_stack == NULL ||
        runtime->secondary_revealer == NULL ||
        runtime->secondary_stack == NULL ||
        runtime->bottom_revealer == NULL ||
        runtime->bottom_stack == NULL ||
        runtime->centre_stack == NULL ||
        runtime->layout_tabs == NULL ||
        runtime->status_bar == NULL) {
        g_object_unref(existing);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    gtk_stack_set_transition_type(
        GTK_STACK(runtime->primary_stack),
        GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_type(
        GTK_STACK(runtime->secondary_stack),
        GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_type(
        GTK_STACK(runtime->bottom_stack),
        GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_type(
        GTK_STACK(runtime->centre_stack),
        GTK_STACK_TRANSITION_TYPE_CROSSFADE);

    gtk_revealer_set_transition_type(
        GTK_REVEALER(runtime->primary_revealer),
        GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT);
    gtk_revealer_set_transition_type(
        GTK_REVEALER(runtime->secondary_revealer),
        GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT);
    gtk_revealer_set_transition_type(
        GTK_REVEALER(runtime->bottom_revealer),
        GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);

    gtk_widget_set_size_request(
        runtime->primary_stack,
        UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH,
        -1);
    gtk_widget_set_size_request(
        runtime->secondary_stack,
        UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH,
        -1);
    gtk_widget_set_size_request(
        runtime->bottom_stack,
        -1,
        UMI_STUDIO_GTK_RUNTIME_BOTTOM_MIN_HEIGHT);
    gtk_revealer_set_child(
        GTK_REVEALER(runtime->primary_revealer), runtime->primary_stack);
    gtk_revealer_set_child(
        GTK_REVEALER(runtime->secondary_revealer), runtime->secondary_stack);
    gtk_revealer_set_child(
        GTK_REVEALER(runtime->bottom_revealer), runtime->bottom_stack);

    workbench = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    runtime->workbench_surface = workbench;
    runtime->editor_command_bar = runtime_build_editor_command_bar(runtime);
    if (runtime->editor_command_bar != NULL) {
        gtk_box_append(GTK_BOX(workbench), runtime->editor_command_bar);
    }
    gtk_widget_set_hexpand(existing, TRUE);
    gtk_widget_set_vexpand(existing, TRUE);
    gtk_box_append(GTK_BOX(workbench), existing);
    g_object_unref(existing);

    runtime->welcome_surface = runtime_build_welcome_surface(runtime);
    if (runtime->welcome_surface == NULL) {
        return UMI_STATUS_OUT_OF_MEMORY;
    }
    gtk_stack_add_named(
        GTK_STACK(runtime->centre_stack),
        runtime->welcome_surface,
        "welcome");
    gtk_stack_add_named(
        GTK_STACK(runtime->centre_stack),
        runtime->workbench_surface,
        "workbench");
    gtk_widget_set_hexpand(runtime->centre_stack, TRUE);
    gtk_widget_set_vexpand(runtime->centre_stack, TRUE);

    gtk_widget_add_css_class(
        runtime->primary_stack, "umicom-runtime-primary");
    gtk_widget_add_css_class(
        runtime->secondary_stack, "umicom-runtime-secondary");
    gtk_widget_add_css_class(
        runtime->bottom_stack, "umicom-runtime-bottom");
    gtk_widget_add_css_class(
        runtime->centre_stack, "umicom-runtime-centre");

    gtk_box_append(
        GTK_BOX(runtime->primary_cluster), runtime->activity_bar);
    gtk_box_append(
        GTK_BOX(runtime->primary_cluster), runtime->primary_revealer);

    runtime_configure_splitter(runtime->primary_paned);
    runtime_configure_splitter(runtime->secondary_paned);
    runtime_configure_splitter(runtime->workspace_paned);
    gtk_paned_set_start_child(
        GTK_PANED(runtime->secondary_paned), runtime->centre_stack);
    gtk_paned_set_end_child(
        GTK_PANED(runtime->secondary_paned), runtime->secondary_revealer);
    gtk_paned_set_start_child(
        GTK_PANED(runtime->primary_paned), runtime->primary_cluster);
    gtk_paned_set_end_child(
        GTK_PANED(runtime->primary_paned), runtime->secondary_paned);
    gtk_paned_set_start_child(
        GTK_PANED(runtime->workspace_paned), runtime->body);
    gtk_paned_set_end_child(
        GTK_PANED(runtime->workspace_paned), runtime->bottom_revealer);
    gtk_widget_set_hexpand(runtime->workspace_paned, TRUE);
    gtk_widget_set_vexpand(runtime->workspace_paned, TRUE);

    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->application_bar);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->toolbar);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->workspace_paned);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->layout_tabs);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->status_bar);

    runtime->layout_locked = 1;
    runtime_workspace_update_lock_button(runtime);
    g_signal_connect(
        runtime->outer_root,
        "map",
        G_CALLBACK(runtime_on_workspace_mapped),
        runtime);
    g_signal_connect(
        runtime->primary_paned,
        "notify::position",
        G_CALLBACK(runtime_on_workspace_position_changed),
        runtime);
    g_signal_connect(
        runtime->secondary_paned,
        "notify::position",
        G_CALLBACK(runtime_on_workspace_position_changed),
        runtime);
    g_signal_connect(
        runtime->workspace_paned,
        "notify::position",
        G_CALLBACK(runtime_on_workspace_position_changed),
        runtime);

    /* A moderate first-run size fits common laptop displays. Users may resize,
     * maximise and save a different geometry through Framework layout state. */
    gtk_window_set_default_size(runtime->owner->window, 1180, 760);
    /* Keep the operating-system title and application bar identity aligned. */
    gtk_window_set_title(runtime->owner->window, "Umicom Studio IDE");
    gtk_window_set_child(runtime->owner->window, runtime->outer_root);
    runtime_welcome_update_mode(runtime);
    return UMI_STATUS_OK;
}
