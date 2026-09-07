/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_layout_tabs.inc
 *
 * PURPOSE:
 *   Provide compact workspace tabs and explicit save, restore and edit-lock
 *   controls inspired by professional IDEs and trading workstations.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Refresh the native selector from the model's named layouts, including blanks.
 * Its notification guard prevents presentation updates from selecting a layout. */
static void runtime_workspace_update_selector(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    GtkStringList *names;
    guint selected = GTK_INVALID_LIST_POSITION;
    size_t index;
    if (model == NULL || runtime->workspace_layout_selector == NULL) return;
    names = gtk_string_list_new(NULL);
    for (index = 0U; index < model->layout_count; ++index) {
        gtk_string_list_append(names, model->layouts[index].name);
        if (strcmp(model->layouts[index].layout_id, model->active_layout_id) == 0)
            selected = (guint)index;
    }
    runtime->syncing_workspace_controls = 1;
    gtk_drop_down_set_model(GTK_DROP_DOWN(runtime->workspace_layout_selector), G_LIST_MODEL(names));
    gtk_drop_down_set_selected(GTK_DROP_DOWN(runtime->workspace_layout_selector), selected);
    runtime->syncing_workspace_controls = 0;
    g_object_unref(names);
}

/* Select an existing named layout only after the current edit is committed. */
static void runtime_on_workspace_selected(GObject *object, GParamSpec *property, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    guint index;
    UmiStatus status;
    (void)property;
    if (model == NULL || runtime->syncing_workspace_controls) return;
    index = gtk_drop_down_get_selected(GTK_DROP_DOWN(object));
    if (index >= model->layout_count) return;
    status = runtime_workspace_select_layout(runtime, model->layouts[index].layout_id);
    if (status != UMI_STATUS_OK) {
        runtime_workspace_update_selector(runtime);
        runtime_show_notification(runtime, "Apply or cancel the current edit before selecting a layout.");
    }
}

/* Give each user-named blank a stable product-qualified key in this session. */
static UmiStatus runtime_on_blank_canvas_named(UmiStudioGtkRuntimeChrome *runtime, const char *name)
{
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    char id[UMI_UI_WORKSPACE_LAYOUT_ID_CAPACITY];
    unsigned int serial;
    size_t index;
    UmiStatus status;
    if (model == NULL || name == NULL || name[0] == '\0') return UMI_STATUS_INVALID_ARGUMENT;
    for (serial = 1U; serial <= UMI_UI_CUSTOM_WORKSPACE_MAX_LAYOUTS; ++serial) {
        (void)snprintf(id, sizeof(id), "umicom.studio.layout.custom-%u", serial);
        for (index = 0U; index < model->layout_count; ++index)
            if (strcmp(model->layouts[index].layout_id, id) == 0) break;
        if (index == model->layout_count) {
            status = runtime_workspace_create_blank(runtime, id, name);
            if (status != UMI_STATUS_OK) runtime_show_notification(runtime, umi_status_text(status));
            return status;
        }
    }
    return UMI_STATUS_CAPACITY_EXCEEDED;
}

/* The actual New canvas control collects a readable name, not a hidden test key. */
static void runtime_on_new_canvas_clicked(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    (void)button;
    if (model == NULL) return;
    if (model->edit_active) {
        runtime_show_notification(runtime, "Apply or cancel this edit before creating another canvas.");
        return;
    }
    (void)runtime_prompt_text(runtime, "New canvas", "Name this canvas. Apply and save it to keep a checkpoint.",
        "Custom canvas", "Create", runtime_on_blank_canvas_named);
}

/* Keep restored/default layouts intact while opening a newly named copy of
 * the current product arrangement. Native rendering succeeds before the live
 * model changes; editor bodies and the saved checkpoint retain their owners. */
static UmiStatus runtime_on_product_default_named(UmiStudioGtkRuntimeChrome *runtime, const char *name)
{
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    UmiUiWorkspaceCustomisation *candidate;
    char id[UMI_UI_WORKSPACE_LAYOUT_ID_CAPACITY];
    unsigned int serial;
    size_t index;
    UmiStatus status;
    if (model == NULL || name == NULL || name[0] == '\0') return UMI_STATUS_INVALID_ARGUMENT;
    if (model->layout_count > UMI_UI_CUSTOM_WORKSPACE_MAX_LAYOUTS) return UMI_STATUS_INVALID_STATE;
    if (model->edit_active) return UMI_STATUS_BUSY;
    for (serial = 1U; serial <= UMI_UI_CUSTOM_WORKSPACE_MAX_LAYOUTS; ++serial) {
        int written = snprintf(id, sizeof(id), "umicom.studio.layout.product-default-%u", serial);
        if (written < 0 || (size_t)written >= sizeof(id)) return UMI_STATUS_CAPACITY_EXCEEDED;
        for (index = 0U; index < model->layout_count; ++index)
            if (strcmp(model->layouts[index].layout_id, id) == 0) break;
        if (index == model->layout_count) break;
    }
    if (serial > UMI_UI_CUSTOM_WORKSPACE_MAX_LAYOUTS) return UMI_STATUS_CAPACITY_EXCEEDED;
    candidate = malloc(sizeof(*candidate));
    if (candidate == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    *candidate = *model;
    status = umi_studio_runtime_workspace_create_default(candidate, id, name);
    if (status == UMI_STATUS_OK) status = runtime_workspace_publish_candidate(runtime, candidate);
    free(candidate);
    if (status != UMI_STATUS_OK) runtime_show_notification(runtime, umi_status_text(status));
    else runtime_show_notification(runtime,
        "New default layout opened. Existing layouts and drafts were kept; use Save layout to save this arrangement.");
    return status;
}

/* An explicit named action avoids resetting a layout restored from storage. */
static void runtime_on_new_default_clicked(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    (void)button;
    if (model == NULL) return;
    if (model->edit_active) {
        runtime_show_notification(runtime, "Apply or cancel this edit before creating a default layout.");
        return;
    }
    (void)runtime_prompt_text(runtime, "New default layout",
        "Name a new copy of the current product layout. Existing layouts and saved checkpoints are kept.",
        "Product default", "Create", runtime_on_product_default_named);
}

/* Compose model selection and explicit edit controls in the existing footer. */
static GtkWidget *runtime_build_layout_tabs(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *profile_strip = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    GtkWidget *scroller = gtk_scrolled_window_new();
    GtkWidget *new_canvas;
    GtkWidget *new_default;
    GtkWidget *manage;
    GtkWidget *restore;
    GtkWidget *save;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bar == NULL || runtime == NULL || profile_strip == NULL ||
        scroller == NULL) {
        return bar;
    }

    gtk_widget_add_css_class(bar, "umicom-runtime-layout-tabs");
    gtk_widget_add_css_class(profile_strip, "umicom-runtime-layout-profiles");

    runtime->workspace_layout_selector = gtk_drop_down_new(NULL, NULL);
    (void)umi_gtk4_automation_tag_widget(runtime->workspace_layout_selector, "studio.workspace.layout-selector");
    g_signal_connect(runtime->workspace_layout_selector, "notify::selected",
        G_CALLBACK(runtime_on_workspace_selected), runtime);
    gtk_box_append(GTK_BOX(profile_strip), runtime->workspace_layout_selector);
    new_canvas = gtk_button_new_with_label("New canvas");
    (void)umi_gtk4_automation_tag_widget(new_canvas, "studio.workspace.new-canvas");
    g_signal_connect(new_canvas, "clicked", G_CALLBACK(runtime_on_new_canvas_clicked), runtime);
    gtk_box_append(GTK_BOX(profile_strip), new_canvas);
    new_default = gtk_button_new_with_label("New default layout");
    (void)umi_gtk4_automation_tag_widget(new_default, "studio.workspace.new-default-layout");
    gtk_widget_set_tooltip_text(new_default,
        "Create a named copy of the current product layout without replacing your existing layouts");
    g_signal_connect(new_default, "clicked", G_CALLBACK(runtime_on_new_default_clicked), runtime);
    gtk_box_append(GTK_BOX(profile_strip), new_default);
    runtime_workspace_update_selector(runtime);

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < sizeof(RUNTIME_LAYOUT_PROFILES) /
                 sizeof(RUNTIME_LAYOUT_PROFILES[0]);
         ++index) {
        const UmiStudioGtkLayoutProfile *profile =
            &RUNTIME_LAYOUT_PROFILES[index];
        GtkWidget *button = runtime_make_command_button(
            runtime,
            profile->label,
            profile->command_id,
            profile->description);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (button != NULL) {
            gtk_widget_add_css_class(button, "flat");
            gtk_box_append(GTK_BOX(profile_strip), button);
        }
    }

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroller), profile_strip);
    gtk_widget_set_hexpand(scroller, TRUE);
    gtk_box_append(GTK_BOX(bar), scroller);

    runtime->layout_lock_button = gtk_toggle_button_new_with_label(
        "Layout Locked");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->layout_lock_button != NULL) {
        gtk_widget_add_css_class(
            runtime->layout_lock_button, "umicom-runtime-layout-lock");
        gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(runtime->layout_lock_button), TRUE);
        g_signal_connect(
            runtime->layout_lock_button,
            "toggled",
            G_CALLBACK(runtime_on_layout_lock_toggled),
            runtime);
        gtk_box_append(GTK_BOX(bar), runtime->layout_lock_button);
    }

    runtime->layout_cancel_button = gtk_button_new_with_label("Cancel Edit");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->layout_cancel_button != NULL) {
        gtk_widget_add_css_class(runtime->layout_cancel_button, "flat");
        gtk_widget_set_visible(runtime->layout_cancel_button, FALSE);
        gtk_widget_set_tooltip_text(
            runtime->layout_cancel_button,
            "Discard panel arrangement changes made since unlocking");
        g_signal_connect(
            runtime->layout_cancel_button,
            "clicked",
            G_CALLBACK(runtime_on_layout_cancel_clicked),
            runtime);
        gtk_box_append(GTK_BOX(bar), runtime->layout_cancel_button);
    }

    manage = runtime_make_local_button(
        runtime,
        "Manage",
        "Browse, apply and inspect workspace layouts",
        1);
    restore = runtime_make_command_button(
        runtime,
        "Restore saved",
        "studio.layout.restore",
        "Restore the last saved layout, or its previous valid checkpoint; open document drafts are kept");
    save = runtime_make_command_button(
        runtime,
        "Save layout",
        "studio.layout.save",
        "Save the applied layout through Framework storage; source files are saved separately");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (manage != NULL) gtk_box_append(GTK_BOX(bar), manage);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (restore != NULL) {
        runtime->workspace_restore_button = restore;
        (void)umi_gtk4_automation_tag_widget(restore, "studio.workspace.restore");
        gtk_box_append(GTK_BOX(bar), restore);
    }
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (save != NULL) {
        runtime->workspace_save_button = save;
        (void)umi_gtk4_automation_tag_widget(save, "studio.workspace.save");
        gtk_box_append(GTK_BOX(bar), save);
    }
    /* Keep storage status visible after a transient notification disappears. */
    runtime->workspace_storage_label = gtk_label_new("");
    gtk_label_set_ellipsize(GTK_LABEL(runtime->workspace_storage_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(runtime->workspace_storage_label), 20);
    gtk_widget_add_css_class(runtime->workspace_storage_label, "dim-label");
    (void)umi_gtk4_automation_tag_widget(runtime->workspace_storage_label, "studio.workspace.storage-status");
    gtk_box_append(GTK_BOX(bar), runtime->workspace_storage_label);
    runtime_workspace_update_storage_controls(runtime);
    return bar;
}
