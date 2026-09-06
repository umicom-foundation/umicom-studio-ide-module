/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_notifications.inc
 *
 * PURPOSE:
 *   Surface command/runtime outcomes without introducing a second notification
 *   service. Framework operations remain authoritative; this is presentation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static gboolean runtime_clear_notification(gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    GtkWidget *label = runtime->notification_label;
    runtime->notification_source = 0U;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (label != NULL) {
        gtk_label_set_text(GTK_LABEL(label), "");
        gtk_widget_set_visible(label, FALSE);
    }
    return G_SOURCE_REMOVE;
}

/*
 * Provide the runtime show notification operation used by this module and its client
 * applications.
 */
static void runtime_show_notification(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *message)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->notification_label == NULL) return;

    gtk_label_set_text(
        GTK_LABEL(runtime->notification_label),
        runtime_safe(message));
    gtk_widget_set_visible(runtime->notification_label, TRUE);

    if (runtime->notification_source != 0U) {
        g_source_remove(runtime->notification_source);
        runtime->notification_source = 0U;
    }
    if (runtime->owner->options.automatic_refresh)
        runtime->notification_source = g_timeout_add(4200U, runtime_clear_notification, runtime);
}

/*
 * Provide the runtime host notify operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_host_notify(void *context, const char *message)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)context;

    runtime_show_notification(runtime, message);
    return UMI_STATUS_OK;
}
