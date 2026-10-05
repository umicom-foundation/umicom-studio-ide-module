/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/editor_recovery.inc
 * PURPOSE: Open the shared local draft recovery workflow while retaining only a weak Studio owner.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/document_recovery.h"
static void RuntimeSourceRecovery(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL)
    {
        runtime_show_notification(runtime, "Could not retain the editor window.");
        return;
    }
    /* Framework owns capture, private storage workers, preview and restoration.
     * Studio supplies its settings namespace and never duplicates source state. */
    UmiStatus status = UmiGtk4AdapterReviewRecovery(runtime->owner->adapter, "Studio", NULL);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && status != UMI_STATUS_OK && status != UMI_STATUS_CANCELLED)
        runtime_show_notification(
            runtime, status == UMI_STATUS_BUSY
                         ? "Finish the current document operation before opening Recovery."
                         : "Recovery could not open. Check that local application storage is available.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
static void RuntimeAppendRecovery(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *bar, GtkWidget *save)
{
    GtkWidget *button = gtk_button_new_with_label("Recovery…");
    (void)umi_gtk4_automation_tag_widget(button, "studio.editor.recovery");
    gtk_widget_set_tooltip_text(button, "Save a private local snapshot of the active draft, preview saved "
                                        "snapshots, and restore one into a new unsaved tab.");
    runtime_bind_editor_action(runtime, button, RuntimeSourceRecovery);
    gtk_box_insert_child_after(GTK_BOX(bar), button, save);
}
