/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_workspace_geometry.inc
 *
 * PURPOSE:
 *   Bind Studio's layout lock, apply and cancel controls to the existing
 *   Framework professional workspace transaction, without private geometry.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Keep native edit controls aligned with the single model transaction. */
static void runtime_workspace_update_lock_button(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->layout_lock_button == NULL) return;
    runtime->syncing_workspace_controls = 1;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(runtime->layout_lock_button),
        runtime->layout_locked != 0);
    gtk_button_set_label(GTK_BUTTON(runtime->layout_lock_button),
        runtime->layout_locked ? "Layout Locked" : "Apply and Lock");
    gtk_widget_set_tooltip_text(runtime->layout_lock_button,
        runtime->layout_locked ? "Unlock to arrange panels; edits can be cancelled" :
        "Apply this arrangement for the current session");
    gtk_widget_remove_css_class(runtime->layout_lock_button, "umicom-layout-locked");
    gtk_widget_remove_css_class(runtime->layout_lock_button, "umicom-layout-editing");
    gtk_widget_add_css_class(runtime->layout_lock_button,
        runtime->layout_locked ? "umicom-layout-locked" : "umicom-layout-editing");
    if (runtime->layout_cancel_button != NULL)
        gtk_widget_set_visible(runtime->layout_cancel_button, runtime->layout_baseline_valid);
    runtime->syncing_workspace_controls = 0;
}

/* Begin or commit the Framework edit; no private geometry baseline exists. */
static void runtime_on_layout_lock_toggled(GtkToggleButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    UmiStatus status;
    if (runtime == NULL || runtime->syncing_workspace_controls) return;
    status = gtk_toggle_button_get_active(button)
        ? runtime_workspace_commit_edit(runtime) : runtime_workspace_begin_edit(runtime);
    if (status != UMI_STATUS_OK) {
        runtime_workspace_update_cache(runtime);
        runtime_show_notification(runtime, "Layout edit could not be changed.");
    }
}

/* Cancel restores the authoritative baseline and all retained panel bodies. */
static void runtime_on_layout_cancel_clicked(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    (void)button;
    if (runtime == NULL) return;
    if (runtime_workspace_cancel_edit(runtime) == UMI_STATUS_OK)
        runtime_show_notification(runtime, "Layout changes were cancelled.");
}
