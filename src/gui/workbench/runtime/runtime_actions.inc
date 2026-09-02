/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_actions.inc
 *
 * PURPOSE:
 *   Route GTK user intent into the authoritative Framework Studio runtime.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_refresh_services(UmiStudioGtkRuntimeChrome *runtime);
/*
 * Provide the runtime show notification operation used by this module and its client
 * applications.
 */
static void runtime_show_notification(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *message);

/*
 * Phase 1 compatibility path:
 * Keep the original direct Framework Studio Runtime dispatch intact as the
 * fallback for every command that does not require GTK-side argument capture.
 * Phase 2 routes only the commands that need native file/folder/text input or
 * a canonical Studio service alias; everything else still follows this exact
 * established execution path.
 */
static UmiStatus runtime_execute_phase1_host_command(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument)
{
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL ||
        runtime->runtime_bootstrap == NULL ||
        command_id == NULL ||
        command_id[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    status = umi_studio_host_controller_execute_command(
        &runtime->host_controller,
        command_id,
        argument);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        char message[384];
        (void)snprintf(
            message,
            sizeof(message),
            "%s: %s",
            command_id,
            umi_status_text(status));
        runtime_show_notification(runtime, message);
        return status;
    }

    return runtime_refresh_services(runtime);
}

/*
 * Provide the runtime execute command operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_execute_command(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument)
{
    return runtime_route_command(runtime, command_id, argument);
}

/*
 * Provide the runtime on command button operation used by this module and its client
 * applications.
 */
static void runtime_on_command_button(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *command_id =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-command-id");
    const char *argument =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-command-argument");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || command_id == NULL) return;
    (void)runtime_execute_command(runtime, command_id, argument);
}

/*
 * Provide the runtime on local command palette operation used by this module and its
 * client applications.
 */
static void runtime_on_local_command_palette(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_show_command_palette((UmiStudioGtkRuntimeChrome *)user_data);
}

/*
 * Provide the runtime on local layout browser operation used by this module and its client
 * applications.
 */
static void runtime_on_local_layout_browser(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_show_layout_browser((UmiStudioGtkRuntimeChrome *)user_data);
}

/*
 * Provide the runtime make local button operation used by this module and its client
 * applications.
 */
static GtkWidget *runtime_make_local_button(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *label,
    const char *tooltip,
    int show_layouts)
{
    GtkWidget *button = gtk_button_new_with_label(label != NULL ? label : "");

    /* The value is consumed by the signal callback selection below. */
    (void)show_layouts;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button == NULL || runtime == NULL) return button;
    /* Stable IDs let UAT select these local controls without using captions. */
    (void)umi_gtk4_automation_tag_widget(
        button,
        show_layouts
            ? "studio.local.layout-browser"
            : "studio.local.command-palette");
    gtk_widget_set_tooltip_text(button, tooltip != NULL ? tooltip : "");
    g_signal_connect(
        button,
        "clicked",
        show_layouts
            ? G_CALLBACK(runtime_on_local_layout_browser)
            : G_CALLBACK(runtime_on_local_command_palette),
        runtime);
    return button;
}

/*
 * Provide the runtime make command button operation used by this module and its client
 * applications.
 */
static GtkWidget *runtime_make_command_button(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *label,
    const char *command_id,
    const char *tooltip)
{
    GtkWidget *button = gtk_button_new_with_label(label != NULL ? label : "");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button == NULL) return NULL;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (tooltip != NULL && tooltip[0] != '\0') {
        gtk_widget_set_tooltip_text(button, tooltip);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (command_id != NULL && command_id[0] != '\0') {
        /* The canonical command ID is also the control's automation address. */
        (void)umi_gtk4_automation_tag_widget(button, command_id);
        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-command-id",
            g_strdup(command_id),
            g_free);
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_command_button),
            runtime);
        gtk_widget_set_sensitive(
            button,
            runtime_command_enabled(runtime, command_id));
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        gtk_widget_set_sensitive(button, FALSE);
    }

    return button;
}

/*
 * Provide the runtime make icon command button operation used by this module and its
 * client applications.
 */
static GtkWidget *runtime_make_icon_command_button(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *icon_name,
    const char *command_id,
    const char *tooltip)
{
    GtkWidget *button =
        gtk_button_new_from_icon_name(icon_name != NULL ? icon_name : "");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button == NULL) return NULL;

    gtk_widget_set_tooltip_text(button, tooltip != NULL ? tooltip : "");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (command_id != NULL && command_id[0] != '\0') {
        /* Icon-only buttons remain discoverable even when their label is hidden. */
        (void)umi_gtk4_automation_tag_widget(button, command_id);
        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-command-id",
            g_strdup(command_id),
            g_free);
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_command_button),
            runtime);
        gtk_widget_set_sensitive(
            button,
            runtime_command_enabled(runtime, command_id));
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        gtk_widget_set_sensitive(button, FALSE);
    }

    return button;
}

/*
 * Provide the runtime refresh command sensitivity operation used by this module and its
 * client applications.
 */
static void runtime_refresh_command_sensitivity(
    UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *container)
{
    GtkWidget *child;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || container == NULL) return;

    child = gtk_widget_get_first_child(container);
    /*
     * Continue only while work remains available; the loop body advances the state on each
     * pass.
     */
    while (child != NULL) {
        const char *command_id =
            (const char *)g_object_get_data(
                G_OBJECT(child),
                "umicom-command-id");

        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (command_id != NULL) {
            gtk_widget_set_sensitive(
                child,
                runtime_command_enabled(runtime, command_id));
        }

        child = gtk_widget_get_next_sibling(child);
    }
}
