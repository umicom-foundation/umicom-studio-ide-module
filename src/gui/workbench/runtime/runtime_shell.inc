/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_shell.inc
 *
 * PURPOSE:
 *   Reframe the existing Framework GTK4 adapter inside a cleaner professional
 *   Studio chrome. The established adapter content is preserved intact and is
 *   used as the central editor/workbench renderer.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_build_shell(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *existing;
    GtkWidget *centre;
    GtkWidget *primary_scroll;
    GtkWidget *secondary_scroll;
    GtkWidget *bottom_scroll;
    UmiStatus status;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /*
     * Studio owns the surrounding professional chrome in this phase.  Ask the
     * reusable GTK4 reference adapter to keep rendering its central editor and
     * workbench content while hiding only its native duplicate chrome.
     */
    status = umi_gtk4_adapter_set_chrome_visibility(
        runtime->owner->adapter,
        0U);
    if (status != UMI_STATUS_OK) return status;

    existing = gtk_window_get_child(runtime->owner->window);
    if (existing == NULL) return UMI_STATUS_INVALID_STATE;

    g_object_ref(existing);
    gtk_window_set_child(runtime->owner->window, NULL);

    runtime->outer_root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    runtime->application_bar = runtime_build_application_bar(runtime);
    runtime->toolbar = runtime_build_toolbar(runtime);
    runtime->body = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    runtime->activity_bar = runtime_build_activity_bar(runtime);
    runtime->primary_revealer = gtk_revealer_new();
    runtime->primary_stack = gtk_stack_new();
    runtime->secondary_revealer = gtk_revealer_new();
    runtime->secondary_stack = gtk_stack_new();
    runtime->bottom_revealer = gtk_revealer_new();
    runtime->bottom_stack = gtk_stack_new();
    runtime->document_tabs = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    runtime->layout_tabs = runtime_build_layout_tabs(runtime);
    runtime->status_bar = runtime_build_status_bar(runtime);

    if (runtime->outer_root == NULL ||
        runtime->application_bar == NULL ||
        runtime->toolbar == NULL ||
        runtime->body == NULL ||
        runtime->activity_bar == NULL ||
        runtime->primary_revealer == NULL ||
        runtime->primary_stack == NULL ||
        runtime->secondary_revealer == NULL ||
        runtime->secondary_stack == NULL ||
        runtime->bottom_revealer == NULL ||
        runtime->bottom_stack == NULL ||
        runtime->document_tabs == NULL ||
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

    gtk_revealer_set_transition_type(
        GTK_REVEALER(runtime->primary_revealer),
        GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT);
    gtk_revealer_set_transition_type(
        GTK_REVEALER(runtime->secondary_revealer),
        GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT);
    gtk_revealer_set_transition_type(
        GTK_REVEALER(runtime->bottom_revealer),
        GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);

    primary_scroll = runtime->primary_stack;
    secondary_scroll = runtime->secondary_stack;
    bottom_scroll = runtime->bottom_stack;

    gtk_widget_set_size_request(
        primary_scroll,
        UMI_STUDIO_GTK_RUNTIME_TOOL_WIDTH,
        -1);
    gtk_widget_set_size_request(
        secondary_scroll,
        UMI_STUDIO_GTK_RUNTIME_TOOL_WIDTH,
        -1);
    gtk_widget_set_size_request(
        bottom_scroll,
        -1,
        UMI_STUDIO_GTK_RUNTIME_BOTTOM_HEIGHT);

    gtk_revealer_set_child(
        GTK_REVEALER(runtime->primary_revealer),
        primary_scroll);
    gtk_revealer_set_child(
        GTK_REVEALER(runtime->secondary_revealer),
        secondary_scroll);
    gtk_revealer_set_child(
        GTK_REVEALER(runtime->bottom_revealer),
        bottom_scroll);

    centre = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    if (centre == NULL) {
        g_object_unref(existing);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    gtk_widget_set_hexpand(centre, TRUE);
    gtk_widget_set_vexpand(centre, TRUE);
    gtk_widget_set_hexpand(existing, TRUE);
    gtk_widget_set_vexpand(existing, TRUE);

    /*
     * Phase 3 adds a compact command bar above the established Framework GTK4
     * editor. It calls DocumentCoordinator operations and never owns document
     * text, undo state or persistence itself.
     */
    runtime->editor_command_bar = runtime_build_editor_command_bar(runtime);
    if (runtime->editor_command_bar != NULL) {
        gtk_box_append(GTK_BOX(centre), runtime->editor_command_bar);
    }

    /*
     * The embedded Framework GTK4 adapter continues to render the authoritative
     * document notebook and editor groups, so no second visible tab strip is
     * added here.  runtime->document_tabs remains a presentation snapshot for
     * diagnostics and future alternate frontends.
     */
    gtk_box_append(GTK_BOX(centre), existing);
    g_object_unref(existing);

    gtk_widget_add_css_class(
        runtime->primary_stack,
        "umicom-runtime-tool-panel");
    gtk_widget_add_css_class(
        runtime->secondary_stack,
        "umicom-runtime-secondary");
    gtk_widget_add_css_class(
        runtime->bottom_stack,
        "umicom-runtime-bottom");

    gtk_box_append(GTK_BOX(runtime->body), runtime->activity_bar);
    gtk_box_append(GTK_BOX(runtime->body), runtime->primary_revealer);
    gtk_box_append(GTK_BOX(runtime->body), centre);
    gtk_box_append(GTK_BOX(runtime->body), runtime->secondary_revealer);

    gtk_widget_set_hexpand(runtime->body, TRUE);
    gtk_widget_set_vexpand(runtime->body, TRUE);

    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->application_bar);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->toolbar);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->body);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->bottom_revealer);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->layout_tabs);
    gtk_box_append(GTK_BOX(runtime->outer_root), runtime->status_bar);

    gtk_window_set_child(
        runtime->owner->window,
        runtime->outer_root);

    return UMI_STATUS_OK;
}
