/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_document_tabs.inc
 *
 * PURPOSE:
 *   Render authoritative DocumentCoordinator working copies as a compact tab
 *   strip above the existing Framework GTK4 editor/workbench renderer.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_on_document_tab(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *view_id =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-view-id");

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL || view_id == NULL) {
        return;
    }

    if (umi_ui_workbench_activate_document(
            umi_studio_ui_workbench(runtime->owner->ui),
            view_id) == UMI_STATUS_OK) {
        (void)runtime_refresh_services(runtime);
    }
}

static UmiStatus runtime_update_document_tabs(
    void *context,
    const UmiStudioRuntimeDocumentTabs *tabs)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)context;
    size_t index;

    if (runtime == NULL || tabs == NULL ||
        runtime->document_tabs == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    runtime_clear_box(runtime->document_tabs);

    if (tabs->count == 0U) {
        GtkWidget *empty = runtime_label("No open documents", 0.0F);
        if (empty != NULL) {
            gtk_widget_add_css_class(empty, "umicom-runtime-muted");
            gtk_box_append(GTK_BOX(runtime->document_tabs), empty);
        }
        return UMI_STATUS_OK;
    }

    for (index = 0U; index < tabs->count; ++index) {
        const UmiStudioRuntimeDocumentTab *tab = &tabs->tabs[index];
        char label[320];
        GtkWidget *button;

        (void)snprintf(
            label,
            sizeof(label),
            "%s%s",
            tab->display_name,
            tab->dirty ? " •" : "");

        button = gtk_button_new_with_label(label);
        if (button == NULL) continue;

        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-view-id",
            g_strdup(tab->view_id),
            g_free);

        gtk_widget_set_tooltip_text(
            button,
            tab->path[0] != '\0' ? tab->path : tab->display_name);

        if (tab->active) {
            gtk_widget_add_css_class(button, "suggested-action");
        }

        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_document_tab),
            runtime);
        gtk_box_append(GTK_BOX(runtime->document_tabs), button);
    }

    return UMI_STATUS_OK;
}
