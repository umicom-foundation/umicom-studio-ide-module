/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/workbench_window.c
 *
 * PURPOSE:
 *   Present the Studio Framework workbench through GTK4 and attach the Framework-owned linked-context control strip.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"

#include <stdlib.h>
#include <string.h>

#include "context_link_centre.h"
#include "umicom/ui/gtk4.h"
#include "umicom/workbench_context_host/gtk4.h"

typedef struct UmiStudioGtkRuntimeChrome UmiStudioGtkRuntimeChrome;

struct UmiStudioGtkWorkbench {
    UmiStudioUi *ui;
    UmiGtk4Adapter *adapter;
    GtkWindow *window;
    UmiStudioContextLinkCentre *context_links;
    GtkWidget *context_strip;
    GtkWidget *content_root;
    UmiStudioGtkRuntimeChrome *runtime_chrome;
};

/*
 * Provide the attach context strip operation used by this module and its client
 * applications.
 */
static UmiStatus attach_context_strip(UmiStudioGtkWorkbench *workbench)
{
    GtkWidget *existing;
    GtkWidget *root;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL || workbench->window == NULL ||
        workbench->context_links == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    existing = gtk_window_get_child(workbench->window);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (existing != NULL) g_object_ref(existing);

    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (root == NULL) {
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (existing != NULL) g_object_unref(existing);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    workbench->context_strip =
        umi_workbench_context_host_gtk4_strip_new(
            umi_studio_context_link_centre_host(workbench->context_links));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench->context_strip == NULL) {
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (existing != NULL) g_object_unref(existing);
        g_object_unref(root);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    gtk_widget_add_css_class(root, "umicom-studio-context-workbench-root");
    gtk_box_append(GTK_BOX(root), workbench->context_strip);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
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


/*
 * Provide the on editor location operation used by this module and its client
 * applications.
 */
static UmiStatus on_editor_location(
    void *context,
    const char *view_id,
    const char *uri,
    const char *language_id,
    uint32_t line,
    uint32_t column,
    uint32_t selection_length,
    uint64_t timestamp_ms)
{
    UmiStudioContextLinkCentre *centre =
        (UmiStudioContextLinkCentre *)context;
    (void)view_id;
    (void)language_id;
    return umi_studio_context_link_centre_publish_source_location(
        centre,
        uri,
        "",
        line,
        column,
        selection_length,
        timestamp_ms);
}

/*
 * Provide the on document activated operation used by this module and its client
 * applications.
 */
static UmiStatus on_document_activated(
    void *context,
    const char *view_id,
    const char *uri,
    const char *language_id,
    uint64_t timestamp_ms)
{
    UmiStudioContextLinkCentre *centre =
        (UmiStudioContextLinkCentre *)context;
    (void)view_id;
    (void)language_id;
    return umi_studio_context_link_centre_publish_selection(
        centre,
        "studio.editor.location",
        "studio.editor",
        uri,
        "document-activated",
        timestamp_ms);
}

/* Find on problem while leaving the underlying catalogue or model owned by this module. */
static UmiStatus on_problem_selected(
    void *context,
    const char *row_text,
    uint64_t timestamp_ms)
{
    return umi_studio_context_link_centre_publish_selection(
        (UmiStudioContextLinkCentre *)context,
        "studio.problems.selection",
        "studio.problems",
        row_text,
        "problem-row",
        timestamp_ms);
}

/*
 * Find on source control while leaving the underlying catalogue or model owned by this
 * module.
 */
static UmiStatus on_source_control_selected(
    void *context,
    const char *view_kind,
    const char *row_text,
    uint64_t timestamp_ms)
{
    return umi_studio_context_link_centre_publish_selection(
        (UmiStudioContextLinkCentre *)context,
        "studio.source-control.selection",
        "studio.source-control",
        row_text,
        view_kind != NULL ? view_kind : "source-control-row",
        timestamp_ms);
}

/* Find on generic while leaving the underlying catalogue or model owned by this module. */
static UmiStatus on_generic_selected(
    void *context,
    const char *source_role,
    const char *subject_id,
    const char *secondary_id,
    uint64_t timestamp_ms)
{
    UmiStudioContextLinkCentre *centre =
        (UmiStudioContextLinkCentre *)context;
    const char *source_id =
        source_role != NULL && source_role[0] != '\0'
        ? source_role
        : "studio.workbench.selection";
    const char *subject =
        subject_id != NULL ? subject_id : "";
    const char *selection_type =
        secondary_id != NULL && secondary_id[0] != '\0'
        ? secondary_id
        : "workbench-selection";

    return umi_studio_context_link_centre_publish_selection(
        centre,
        source_id,
        "studio.workbench.observer",
        subject,
        selection_type,
        timestamp_ms);
}

/*
 * Provide the bind context interactions operation used by this module and its client
 * applications.
 */
static UmiStatus bind_context_interactions(
    UmiStudioGtkWorkbench *workbench)
{
    UmiGtk4ContextInteractionSink sink;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL || workbench->adapter == NULL ||
        workbench->context_links == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    (void)memset(&sink, 0, sizeof(sink));
    sink.structure_size = (uint32_t)sizeof(sink);
    sink.context = workbench->context_links;
    sink.editor_location = on_editor_location;
    sink.document_activated = on_document_activated;
    sink.problem_selected = on_problem_selected;
    sink.source_control_selected = on_source_control_selected;
    sink.generic_selected = on_generic_selected;

    return umi_gtk4_adapter_bind_context_interactions(
        workbench->adapter,
        &sink);
}

/*
 * The established GTK4 workbench source remains the executable's registered
 * translation unit. The private Studio Runtime fragments below extend this host
 * additively so the existing CMake target, adapter lifecycle and context-link
 * behavior are preserved verbatim.
 */
#include "runtime/runtime_unity.inc"

/*
 * Initialise studio gtk workbench from caller-provided values so later operations receive
 * a known state.
 */
UmiStatus umi_studio_gtk_workbench_create(
    GtkApplication *application,
    UmiStudioUi *ui,
    UmiDesktopShellModel *desktop_shell,
    UmiStudioGtkWorkbench **out_workbench)
{
    UmiStudioGtkWorkbench *workbench;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (application == NULL || ui == NULL || desktop_shell == NULL ||
        out_workbench == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_workbench = NULL;

    workbench = (UmiStudioGtkWorkbench *)calloc(1U, sizeof(*workbench));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    workbench->ui = ui;
    status = umi_gtk4_adapter_create(application, &workbench->adapter);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_gtk4_adapter_bind_desktop_shell(
            workbench->adapter, desktop_shell);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_gtk4_adapter_present(
            workbench->adapter,
            umi_studio_ui_shell(ui));
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        workbench->window = GTK_WINDOW(
            umi_gtk4_adapter_native_window(workbench->adapter));
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (workbench->window == NULL) {
            status = UMI_STATUS_INVALID_STATE;
        }
    }

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_context_link_centre_create(
            umi_studio_ui_workbench(ui),
            umi_studio_services_session(
                umi_studio_ui_services(ui)),
            &workbench->context_links);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = bind_context_interactions(workbench);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_context_link_centre_refresh(
            workbench->context_links,
            (uint64_t)(g_get_monotonic_time() / 1000));
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = attach_context_strip(workbench);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = runtime_attach(workbench);
    }

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_gtk_workbench_destroy(workbench);
        return status;
    }

    *out_workbench = workbench;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio gtk workbench so the same storage can be reused
 * safely.
 */
void umi_studio_gtk_workbench_destroy(
    UmiStudioGtkWorkbench *workbench)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL) return;

    runtime_detach(workbench);

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

/*
 * Provide the studio gtk workbench window operation used by this module and its client
 * applications.
 */
GtkWindow *umi_studio_gtk_workbench_window(
    UmiStudioGtkWorkbench *workbench)
{
    return workbench != NULL ? workbench->window : NULL;
}

/*
 * Provide the studio gtk workbench refresh operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_gtk_workbench_refresh(
    UmiStudioGtkWorkbench *workbench)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    status = umi_studio_ui_refresh(workbench->ui);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = umi_studio_context_link_centre_refresh(
        workbench->context_links,
        (uint64_t)(g_get_monotonic_time() / 1000));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = umi_gtk4_adapter_refresh(workbench->adapter);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    return umi_workbench_context_host_gtk4_strip_refresh(
        workbench->context_strip,
        umi_studio_context_link_centre_host(workbench->context_links));
}
