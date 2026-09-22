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

#include "umicom/studio/test_execution.h"

/* Do not enter synchronous service destruction with a live discovery payload.
 * The ordinary refresh loop reaps Stop. No discarded edits or automatic close
 * are implied: after it completes the user requests close again. */
static int RuntimeStopDiscoveryBeforeClose(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL) return 0;
    UmiStudioTestService *tests = umi_studio_services_tests(
        umi_studio_ui_services(runtime->owner->ui));
    if (UmiStudioTestRunPending(tests)) {
        (void)UmiStudioTestRunCancel(tests);
        runtime_show_notification(runtime,
            "Stopping the test run. Close again after it finishes; your documents remain open.");
        return 1;
    }
    if (!UmiStudioTestDiscoveryPending(tests)) return 0;
    (void)umi_studio_test_service_stop(tests);
    runtime_show_notification(runtime,
        "Stopping test discovery. Close again after it finishes; your documents remain open.");
    return 1;
}

static void runtime_close_confirmed(GtkButton *button, gpointer user_data)
{
    UmiStudioFileDialogRequest *request = user_data;
    GObject *ownerWindow = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &ownerWindow);
    GtkWidget *dialog =
        GTK_WIDGET(gtk_widget_get_root(GTK_WIDGET(button)));

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        if (GTK_IS_WINDOW(dialog)) gtk_window_destroy(GTK_WINDOW(dialog));
        g_clear_object(&ownerWindow);
        return;
    }

    if (RuntimeStopDiscoveryBeforeClose(runtime)) {
        if (GTK_IS_WINDOW(dialog)) gtk_window_destroy(GTK_WINDOW(dialog));
        g_clear_object(&ownerWindow);
        return;
    }
    runtime->force_close = 1;

    /* Apply this branch only when its contract condition is satisfied. */
    if (GTK_IS_WINDOW(dialog)) {
        gtk_window_destroy(GTK_WINDOW(dialog));
    }
    /* Keep the owner alive through both destroys; the dialog owns request. */
    gtk_window_destroy(GTK_WINDOW(ownerWindow));
    g_clear_object(&ownerWindow);
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
    UmiStudioFileDialogRequest *request;
    char explanation[384];

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

    request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) {
        gtk_window_destroy(GTK_WINDOW(dialog));
        runtime_show_notification(runtime, "Unable to show close confirmation. Your window remains open.");
        return;
    }
    g_object_set_data_full(G_OBJECT(dialog), "umicom-close-owner", request,
        (GDestroyNotify)runtime_file_dialog_request_free);
    gtk_window_set_title(GTK_WINDOW(dialog), "Close Umicom Studio?");
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
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
    (void)snprintf(explanation, sizeof(explanation),
        "%zu document(s) have unsaved changes.%s Cancel to return to your work. "
        "Close Anyway discards unsaved edits and ends this Studio session.",
        report->dirty_document_count,
        report->decision == UMI_STUDIO_CLOSE_CONFIRM_RUNNING ||
        report->decision == UMI_STUDIO_CLOSE_CONFIRM_DIRTY_AND_RUNNING
            ? " A build or another operation is still running." : "");
    gtk_box_append(GTK_BOX(root), runtime_label(explanation, 0.0F));

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
        request);

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
    if (runtime == NULL) return TRUE;
    if (RuntimeStopDiscoveryBeforeClose(runtime)) return TRUE;
    if (runtime->force_close) return FALSE;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->runtime_bootstrap == NULL ||
        runtime->runtime_bootstrap->platform == NULL) {
        runtime_show_notification(runtime, "Unable to check open documents. Studio has kept this window open.");
        return TRUE;
    }

    status = umi_studio_runtime_platform_snapshot(
        runtime->runtime_bootstrap->platform,
        &snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, "Unable to check unsaved or running work. Studio has kept this window open.");
        return TRUE;
    }

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bindings == NULL || bindings->documents == NULL) {
        runtime_show_notification(runtime, "The document service is unavailable. Studio has kept this window open.");
        return TRUE;
    }

    status = UmiStudioCloseGuardEvaluateWithActivity(
        bindings->documents,
        &snapshot.ide.context,
        UmiStudioBuildBusy(umi_studio_services_build(
            umi_studio_ui_services(runtime->owner->ui))) ||
            runtime->debugger_snapshot.session_count > 0U,
        &report);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, "Unable to check unsaved or running work. Studio has kept this window open.");
        return TRUE;
    }

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
