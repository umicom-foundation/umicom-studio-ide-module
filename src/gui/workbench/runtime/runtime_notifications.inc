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

/* Clearing feedback now retains the widget across notifications that may close its host. The earlier borrowed-widget implementation remains for review. */
#if 0
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
#endif
static gboolean runtime_clear_notification(gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    GtkWidget *label = runtime->notification_label;
    runtime->notification_source = 0U;
    if (label != NULL) {
        /* Changing a label may notify an observer which closes the workbench.
         * Keep the widget alive independently and do not read runtime again. */
        g_object_ref(label);
        gtk_label_set_text(GTK_LABEL(label), "");
        gtk_widget_set_visible(label, FALSE);
        g_object_unref(label);
    }
    return G_SOURCE_REMOVE;
}

/*
 * Provide the runtime show notification operation used by this module and its client
 * applications.
 */
/* Feedback now preserves its widget lifetime and records a presentation revision. This prevents fallback errors replacing useful recovery guidance; the previous implementation is retained. */
#if 0
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
#endif
static void runtime_show_notification(UmiStudioGtkRuntimeChrome *runtime, const char *message)
{
    if (runtime == NULL || runtime->notification_label == NULL) return;
    GtkWidget *label = g_object_ref(runtime->notification_label);
    char *text = g_strdup(runtime_safe(message));
    if (runtime->notification_source != 0U) {
        g_source_remove(runtime->notification_source);
        runtime->notification_source = 0U;
    }
    ++runtime->notification_revision;
    if (runtime->notification_revision == 0U) ++runtime->notification_revision;
    if (runtime->owner->options.automatic_refresh)
        runtime->notification_source = g_timeout_add(4200U, runtime_clear_notification, runtime);
    /* Set all host-owned state before GTK signals can close the workbench.
     * Teardown removes its timer; only the retained label is used afterwards. */
    (void)umi_gtk4_automation_tag_widget(label, "studio.notification");
    gtk_label_set_text(GTK_LABEL(label), text);
    gtk_widget_set_visible(label, TRUE);
    g_free(text);
    g_object_unref(label);
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
