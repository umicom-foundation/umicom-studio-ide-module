/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_close_guard.inc
 *
 * PURPOSE:
 *   Present Framework close-safety evidence before Studio exits. The dialog is
 *   GTK-only; dirty-document and running-operation rules remain Framework-owned.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_close_confirmed(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    GtkWidget *dialog =
        GTK_WIDGET(gtk_widget_get_root(GTK_WIDGET(button)));

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return;
    }

    runtime->force_close = 1;

    /* Apply this branch only when its contract condition is satisfied. */
    if (GTK_IS_WINDOW(dialog)) {
        gtk_window_destroy(GTK_WINDOW(dialog));
    }
    gtk_window_destroy(runtime->owner->window);
}

/*
 * Provide the runtime close cancelled operation used by this module and its client
 * applications.
 */
static void runtime_close_cancelled(GtkButton *button, gpointer user_data)
{
    GtkWidget *dialog =
        GTK_WIDGET(gtk_widget_get_root(GTK_WIDGET(button)));
    (void)user_data;

    /* Apply this branch only when its contract condition is satisfied. */
    if (GTK_IS_WINDOW(dialog)) {
        gtk_window_destroy(GTK_WINDOW(dialog));
    }
}

/*
 * Provide the runtime show close confirmation operation used by this module and its client
 * applications.
 */
static void runtime_show_close_confirmation(
    UmiStudioGtkRuntimeChrome *runtime,
    const UmiStudioRuntimeCloseReport *report)
{
    GtkWidget *dialog;
    GtkWidget *root;
    GtkWidget *buttons;
    GtkWidget *cancel;
    GtkWidget *close_anyway;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || report == NULL ||
        runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return;
    }

    dialog = GTK_WIDGET(gtk_window_new());
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    cancel = gtk_button_new_with_label("Cancel");
    close_anyway = gtk_button_new_with_label("Close Anyway");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (dialog == NULL || root == NULL || buttons == NULL ||
        cancel == NULL || close_anyway == NULL) {
        return;
    }

    gtk_window_set_title(GTK_WINDOW(dialog), "Close Umicom Studio?");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(
        GTK_WINDOW(dialog),
        runtime->owner->window);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 480, 220);

    gtk_widget_set_margin_top(root, 16);
    gtk_widget_set_margin_bottom(root, 16);
    gtk_widget_set_margin_start(root, 16);
    gtk_widget_set_margin_end(root, 16);

    gtk_box_append(
        GTK_BOX(root),
        runtime_section_title("Unsaved or running work requires confirmation"));
    gtk_box_append(
        GTK_BOX(root),
        runtime_label(report->summary, 0.0F));

    gtk_widget_set_halign(buttons, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(buttons), cancel);
    gtk_box_append(GTK_BOX(buttons), close_anyway);
    gtk_box_append(GTK_BOX(root), buttons);
    gtk_window_set_child(GTK_WINDOW(dialog), root);

    g_signal_connect(
        cancel,
        "clicked",
        G_CALLBACK(runtime_close_cancelled),
        runtime);
    g_signal_connect(
        close_anyway,
        "clicked",
        G_CALLBACK(runtime_close_confirmed),
        runtime);

    gtk_window_present(GTK_WINDOW(dialog));
}

/*
 * Provide the runtime on close request operation used by this module and its client
 * applications.
 */
static gboolean runtime_on_close_request(
    GtkWindow *window,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStudioRuntimeSnapshot snapshot;
    UmiStudioRuntimeBindings *bindings;
    UmiStudioRuntimeCloseReport report;
    UmiStatus status;

    (void)window;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->force_close) return FALSE;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->runtime_bootstrap == NULL ||
        runtime->runtime_bootstrap->platform == NULL) {
        return FALSE;
    }

    status = umi_studio_runtime_platform_snapshot(
        runtime->runtime_bootstrap->platform,
        &snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return FALSE;

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bindings == NULL || bindings->documents == NULL) return FALSE;

    status = umi_studio_close_guard_evaluate(
        bindings->documents,
        &snapshot.ide.context,
        &report);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return FALSE;

    /*
     * The older Studio debugger wrapper and the new DAP Runtime use different
     * facade types today. Preserve the Framework close decision and supplement
     * it with the existing authoritative Studio debugger snapshot until those
     * facades converge.
     */
    if (runtime->debugger_snapshot.session_count > 0U) {
        report.debug_running = 1;
        /* Apply this branch only when its contract condition is satisfied. */
        if (report.dirty_document_count > 0U) {
            report.decision = UMI_STUDIO_CLOSE_CONFIRM_DIRTY_AND_RUNNING;
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            report.decision = UMI_STUDIO_CLOSE_CONFIRM_RUNNING;
        }
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (report.decision == UMI_STUDIO_CLOSE_ALLOW) return FALSE;

    runtime_show_close_confirmation(runtime, &report);
    return TRUE;
}
