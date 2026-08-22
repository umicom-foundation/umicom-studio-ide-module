/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/workbench_window.c
 *
 * PURPOSE:
 *   Present the Studio Framework workbench through GTK4 and attach the Framework-owned linked-context control strip.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"

#include <stdlib.h>

#include "context_link_centre.h"
#include "umicom/ui/gtk4.h"
#include "umicom/workbench_context_host/gtk4.h"

struct UmiStudioGtkWorkbench {
    UmiStudioUi *ui;
    UmiGtk4Adapter *adapter;
    GtkWindow *window;
    UmiStudioContextLinkCentre *context_links;
    GtkWidget *context_strip;
    GtkWidget *content_root;
};

static UmiStatus attach_context_strip(UmiStudioGtkWorkbench *workbench)
{
    GtkWidget *existing;
    GtkWidget *root;

    if (workbench == NULL || workbench->window == NULL ||
        workbench->context_links == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    existing = gtk_window_get_child(workbench->window);
    if (existing != NULL) g_object_ref(existing);

    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    if (root == NULL) {
        if (existing != NULL) g_object_unref(existing);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    workbench->context_strip =
        umi_workbench_context_host_gtk4_strip_new(
            umi_studio_context_link_centre_host(workbench->context_links));
    if (workbench->context_strip == NULL) {
        if (existing != NULL) g_object_unref(existing);
        g_object_unref(root);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    gtk_widget_add_css_class(root, "umicom-studio-context-workbench-root");
    gtk_box_append(GTK_BOX(root), workbench->context_strip);

    if (existing != NULL) {
        gtk_window_set_child(workbench->window, NULL);
        gtk_widget_set_hexpand(existing, TRUE);
        gtk_widget_set_vexpand(existing, TRUE);
        gtk_box_append(GTK_BOX(root), existing);
        g_object_unref(existing);
    }

    gtk_window_set_child(workbench->window, root);
    workbench->content_root = root;
    return UMI_STATUS_OK;
}

UmiStatus umi_studio_gtk_workbench_create(
    GtkApplication *application,
    UmiStudioUi *ui,
    UmiDesktopShellModel *desktop_shell,
    UmiStudioGtkWorkbench **out_workbench)
{
    UmiStudioGtkWorkbench *workbench;
    UmiStatus status;

    if (application == NULL || ui == NULL || desktop_shell == NULL ||
        out_workbench == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_workbench = NULL;

    workbench = (UmiStudioGtkWorkbench *)calloc(1U, sizeof(*workbench));
    if (workbench == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    workbench->ui = ui;
    status = umi_gtk4_adapter_create(application, &workbench->adapter);
    if (status == UMI_STATUS_OK) {
        status = umi_gtk4_adapter_bind_desktop_shell(
            workbench->adapter, desktop_shell);
    }
    if (status == UMI_STATUS_OK) {
        status = umi_gtk4_adapter_present(
            workbench->adapter,
            umi_studio_ui_shell(ui));
    }
    if (status == UMI_STATUS_OK) {
        workbench->window = GTK_WINDOW(
            umi_gtk4_adapter_native_window(workbench->adapter));
        if (workbench->window == NULL) {
            status = UMI_STATUS_INVALID_STATE;
        }
    }

    if (status == UMI_STATUS_OK) {
        status = umi_studio_context_link_centre_create(
            umi_studio_ui_workbench(ui),
            umi_studio_services_session(
                umi_studio_ui_services(ui)),
            &workbench->context_links);
    }
    if (status == UMI_STATUS_OK) {
        status = umi_studio_context_link_centre_refresh(
            workbench->context_links,
            (uint64_t)(g_get_monotonic_time() / 1000));
    }
    if (status == UMI_STATUS_OK) {
        status = attach_context_strip(workbench);
    }

    if (status != UMI_STATUS_OK) {
        umi_studio_gtk_workbench_destroy(workbench);
        return status;
    }

    *out_workbench = workbench;
    return UMI_STATUS_OK;
}

void umi_studio_gtk_workbench_destroy(
    UmiStudioGtkWorkbench *workbench)
{
    if (workbench == NULL) return;

    /*
     * Destroy GTK first so signal closures carrying the borrowed host pointer
     * disappear before the toolkit-neutral context centre is released.
     */
    umi_gtk4_adapter_destroy(workbench->adapter);
    workbench->adapter = NULL;
    workbench->window = NULL;
    workbench->context_strip = NULL;
    workbench->content_root = NULL;

    umi_studio_context_link_centre_destroy(workbench->context_links);
    workbench->context_links = NULL;
    free(workbench);
}

GtkWindow *umi_studio_gtk_workbench_window(
    UmiStudioGtkWorkbench *workbench)
{
    return workbench != NULL ? workbench->window : NULL;
}

UmiStatus umi_studio_gtk_workbench_refresh(
    UmiStudioGtkWorkbench *workbench)
{
    UmiStatus status;
    if (workbench == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    status = umi_studio_ui_refresh(workbench->ui);
    if (status != UMI_STATUS_OK) return status;

    status = umi_studio_context_link_centre_refresh(
        workbench->context_links,
        (uint64_t)(g_get_monotonic_time() / 1000));
    if (status != UMI_STATUS_OK) return status;

    status = umi_gtk4_adapter_refresh(workbench->adapter);
    if (status != UMI_STATUS_OK) return status;

    return umi_workbench_context_host_gtk4_strip_refresh(
        workbench->context_strip,
        umi_studio_context_link_centre_host(workbench->context_links));
}
