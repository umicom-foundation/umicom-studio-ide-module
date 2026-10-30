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
#include "umicom/ui/gtk4/workstation/tab_strip.h"

/* The previous full row rebuild interrupted pointer gestures and keyboard focus
 * on the periodic refresh. Framework now retains keyed controls and scrolls
 * overflow. Keep the old rendering and activation paths for engineering review. */
#if 0


/* Selecting a source tab now returns typing focus to its retained editor and reveals a hidden panel. Preserve the prior activation-only route for review. */
#if 0
static void runtime_on_document_tab(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *view_id =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-view-id");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL || view_id == NULL) {
        return;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_ui_workbench_activate_document(
            umi_studio_ui_workbench(runtime->owner->ui),
            view_id) == UMI_STATUS_OK) {
        (void)runtime_refresh_services(runtime);
    }
}

#endif
static void runtime_on_document_tab(GtkButton *button, gpointer userData)
{
    UmiStudioGtkRuntimeChrome *runtime = userData;
    const char *viewId = g_object_get_data(G_OBJECT(button), "umicom-view-id");
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL || viewId == NULL) return;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return;
    char *selectedId = g_strdup(viewId);
    UmiStatus status = umi_ui_workbench_activate_document(
        umi_studio_ui_workbench(runtime->owner->ui), selectedId);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && status == UMI_STATUS_OK) (void)RuntimePresentActiveSource(runtime);
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    g_free(selectedId);
}

/*
 * Provide the runtime update document tabs operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_update_document_tabs(
    void *context,
    const UmiStudioRuntimeDocumentTabs *tabs)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)context;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || tabs == NULL ||
        runtime->document_tabs == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    runtime_clear_box(runtime->document_tabs);

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (tabs->count == 0U) {
        GtkWidget *empty = runtime_label("No open documents", 0.0F);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (empty != NULL) {
            gtk_widget_add_css_class(empty, "umicom-runtime-muted");
            gtk_box_append(GTK_BOX(runtime->document_tabs), empty);
        }
        return UMI_STATUS_OK;
    }

    /* Visit each bounded item once so every record receives the same rule. */
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
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (button == NULL) continue;

        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-view-id",
            g_strdup(tab->view_id),
            g_free);

        gtk_widget_set_tooltip_text(
            button,
            tab->path[0] != '\0' ? tab->path : tab->display_name);

        /* Apply this operation only while the related capability or state is available. */
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

#endif

/* The strip owns a weak window/generation request, not the runtime address.
 * An old retained tab therefore cannot activate a replacement or closed host. */
static void RuntimeTabRequestFree(gpointer data)
{
    runtime_file_dialog_request_free(data);
}
static void RuntimeActivateDocumentTab(const char *viewId, void *context)
{
    UmiStudioFileDialogRequest *request = context;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && runtime->owner != NULL && runtime->owner->ui != NULL) {
        UmiStatus status = umi_ui_workbench_activate_document(
            umi_studio_ui_workbench(runtime->owner->ui), viewId);
        runtime = RuntimeResolveOwner(request, &window);
        if (runtime != NULL && status == UMI_STATUS_OK)
            (void)RuntimePresentActiveSource(runtime);
    }
    g_clear_object(&window);
}
static UmiStatus runtime_update_document_tabs(void *context,
    const UmiStudioRuntimeDocumentTabs *tabs)
{
    UmiStudioGtkRuntimeChrome *runtime = context;
    if (runtime == NULL || tabs == NULL || runtime->document_tabs == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    if (tabs->count > UMI_STUDIO_DOCUMENT_TAB_CAPACITY || tabs->count > UMI_GTK4_TAB_STRIP_MAX)
        return UMI_STATUS_CAPACITY_EXCEEDED;
    const size_t count = tabs->count;
    UmiGtk4TabStripItem *items = g_new0(UmiGtk4TabStripItem, count);
    for (size_t index = 0U; index < count; ++index) {
        const UmiStudioRuntimeDocumentTab *tab = &tabs->tabs[index];
        items[index] = (UmiGtk4TabStripItem){g_strdup(tab->view_id),
            g_strdup_printf("%s%s", tab->display_name, tab->dirty ? " ●" : ""),
            g_strdup(tab->path[0] != '\0' ? tab->path : tab->display_name), tab->active};
    }
    /* Sync owns its native lifetime across notifications. Do not access runtime
     * afterwards: a host observer can close Studio during reconciliation.
     * Keep identifiers and count independent of the caller snapshot as well. */
    GtkWidget *strip = g_object_ref(runtime->document_tabs);
    UmiStatus status = UmiGtk4TabStripSync(strip, items, count);
    if (status == UMI_STATUS_OK) {
        for (size_t index = 0U; index < count; ++index) {
            GtkWidget *button = UmiGtk4TabStripFind(strip, items[index].id);
            if (button != NULL && g_object_get_data(G_OBJECT(button), "umicom-view-id") == NULL)
                g_object_set_data_full(G_OBJECT(button), "umicom-view-id", g_strdup(items[index].id), g_free);
        }
    }
    for (size_t index = 0U; index < count; ++index) {
        g_free((void *)items[index].id);
        g_free((void *)items[index].title);
        g_free((void *)items[index].tooltip);
    }
    g_free(items);
    g_object_unref(strip);
    return status;
}
