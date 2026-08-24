/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_notifications.inc
 *
 * PURPOSE:
 *   Surface command/runtime outcomes without introducing a second notification
 *   service. Framework operations remain authoritative; this is presentation.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static gboolean runtime_clear_notification(gpointer user_data)
{
    GtkWidget *label = GTK_WIDGET(user_data);

    if (label != NULL) {
        gtk_label_set_text(GTK_LABEL(label), "");
        gtk_widget_set_visible(label, FALSE);
    }
    return G_SOURCE_REMOVE;
}

static void runtime_show_notification(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *message)
{
    if (runtime == NULL || runtime->notification_label == NULL) return;

    gtk_label_set_text(
        GTK_LABEL(runtime->notification_label),
        runtime_safe(message));
    gtk_widget_set_visible(runtime->notification_label, TRUE);

    (void)g_timeout_add(
        4200U,
        runtime_clear_notification,
        runtime->notification_label);
}

static UmiStatus runtime_host_notify(void *context, const char *message)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)context;

    runtime_show_notification(runtime, message);
    return UMI_STATUS_OK;
}
