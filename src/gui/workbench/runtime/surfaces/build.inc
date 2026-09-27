/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/build.inc
 *
 * PURPOSE:
 *   Build the Framework-backed build presentation surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/build/review_gtk4.h"

/* Studio selects the existing producer. Framework immediately copies the
 * history and owns the review window; it never retains Studio's runtime. */
static void RuntimeOpenBuildHistoryReview(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    if (runtime == NULL || runtime->owner == NULL) return;
    UmiStudioBuildService *service = umi_studio_services_build(
        umi_studio_ui_services(runtime->owner->ui));
    GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(button));
    if (service != NULL && GTK_IS_WINDOW(root)) {
        (void)UmiGtk4BuildReviewPresent(GTK_WINDOW(root),
            umi_studio_build_service_history(service));
    }
}

static GtkWidget *runtime_build_surface_build(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Build");
    UmiStudioBuildService *service;
    UmiStudioBuildSnapshot snapshot;
    char value[128];

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || runtime == NULL || runtime->owner == NULL) return panel;

    service = umi_studio_services_build(
        umi_studio_ui_services(runtime->owner->ui));

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service != NULL &&
        umi_studio_build_service_snapshot(
            service,
            &snapshot) == UMI_STATUS_OK) {
        runtime_panel_add_stat(panel, "Profile", snapshot.profile_id);
        runtime_panel_add_stat(panel, "Build directory", snapshot.build_directory);
        (void)snprintf(value, sizeof(value), "%zu", snapshot.diagnostic_count);
        runtime_panel_add_stat(panel, "Diagnostics", value);
        (void)snprintf(value, sizeof(value), "%zu", snapshot.artifact_count);
        runtime_panel_add_stat(panel, "Artifacts", value);
        runtime_panel_add_stat(
            panel,
            "Last status",
            umi_status_text(snapshot.last_status));
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        runtime_panel_add_stat(panel, "Build service", "Unavailable");
    }

    {
        static const char *const labels[] = {
            "Project Settings", "Configure", "Build", "Test", "Run", "Install", "Cancel", "Clean", "Package"
        };
        static const char *const commands[] = {
            "build.settings", "build.configure", "build.build", "build.test", "run.start",
            "build.install", "build.cancel", "build.clean", "build.package"
        };
        runtime_panel_add_commands(runtime, panel, labels, commands, 9U);
    }
    /* Keep every existing build action. Reviewing never submits another job. */
    GtkWidget *review_button = gtk_button_new_with_label("Review build history");
    gtk_widget_set_tooltip_text(review_button,
        "Inspect copied outcomes and problems without starting an operation");
    runtime_bind_editor_action(runtime, review_button, RuntimeOpenBuildHistoryReview);
    gtk_box_append(GTK_BOX(panel), review_button);

    runtime->build_progress_label = runtime_label("Ready.", 0.0F);
    gtk_box_append(GTK_BOX(panel), runtime->build_progress_label);
    RuntimeRefreshBuildOutput(runtime);

    runtime_append_context_actions(runtime, panel, "build");

    return runtime_scroll(panel);
}

