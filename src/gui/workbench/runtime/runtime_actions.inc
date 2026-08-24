/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_actions.inc
 *
 * PURPOSE:
 *   Route GTK user intent into the authoritative Framework Studio runtime.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_refresh_services(UmiStudioGtkRuntimeChrome *runtime);
static void runtime_show_notification(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *message);

static UmiStatus runtime_execute_command(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument)
{
    UmiStatus status;

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

    if (runtime == NULL || command_id == NULL) return;
    (void)runtime_execute_command(runtime, command_id, NULL);
}

static void runtime_on_local_command_palette(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_show_command_palette((UmiStudioGtkRuntimeChrome *)user_data);
}

static void runtime_on_local_layout_browser(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_show_layout_browser((UmiStudioGtkRuntimeChrome *)user_data);
}

static GtkWidget *runtime_make_local_button(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *label,
    const char *tooltip,
    int show_layouts)
{
    GtkWidget *button = gtk_button_new_with_label(label != NULL ? label : "");

    /* The value is consumed by the signal callback selection below. */
    (void)show_layouts;

    if (button == NULL || runtime == NULL) return button;
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

static GtkWidget *runtime_make_command_button(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *label,
    const char *command_id,
    const char *tooltip)
{
    GtkWidget *button = gtk_button_new_with_label(label != NULL ? label : "");

    if (button == NULL) return NULL;

    if (tooltip != NULL && tooltip[0] != '\0') {
        gtk_widget_set_tooltip_text(button, tooltip);
    }

    if (command_id != NULL && command_id[0] != '\0') {
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
    } else {
        gtk_widget_set_sensitive(button, FALSE);
    }

    return button;
}

static GtkWidget *runtime_make_icon_command_button(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *icon_name,
    const char *command_id,
    const char *tooltip)
{
    GtkWidget *button =
        gtk_button_new_from_icon_name(icon_name != NULL ? icon_name : "");

    if (button == NULL) return NULL;

    gtk_widget_set_tooltip_text(button, tooltip != NULL ? tooltip : "");

    if (command_id != NULL && command_id[0] != '\0') {
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
    } else {
        gtk_widget_set_sensitive(button, FALSE);
    }

    return button;
}

static void runtime_refresh_command_sensitivity(
    UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *container)
{
    GtkWidget *child;

    if (runtime == NULL || container == NULL) return;

    child = gtk_widget_get_first_child(container);
    while (child != NULL) {
        const char *command_id =
            (const char *)g_object_get_data(
                G_OBJECT(child),
                "umicom-command-id");

        if (command_id != NULL) {
            gtk_widget_set_sensitive(
                child,
                runtime_command_enabled(runtime, command_id));
        }

        child = gtk_widget_get_next_sibling(child);
    }
}
