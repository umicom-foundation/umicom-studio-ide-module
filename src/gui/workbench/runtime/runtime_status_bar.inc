/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_status_bar.inc
 *
 * PURPOSE:
 *   Render the Framework Studio status model as a compact professional status
 *   strip. Existing Studio source-control/test/debug snapshots supplement the
 *   IDE-integration fields where those older adapters use different contracts.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_status_bar(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    size_t index;

    if (bar == NULL || runtime == NULL) return bar;

    gtk_widget_add_css_class(bar, "umicom-runtime-status");

    for (index = 0U;
         index < sizeof(RUNTIME_STATUS_PROFILES) /
                 sizeof(RUNTIME_STATUS_PROFILES[0]);
         ++index) {
        const UmiStudioGtkStatusProfile *profile =
            &RUNTIME_STATUS_PROFILES[index];
        GtkWidget *label = gtk_label_new("");

        if (label == NULL) continue;

        runtime->status_labels[profile->kind] = label;
        gtk_widget_set_tooltip_text(label, profile->label);
        gtk_widget_add_css_class(label, "umicom-runtime-muted");
        gtk_box_append(GTK_BOX(bar), label);
    }

    return bar;
}

static void runtime_status_override_legacy_services(
    UmiStudioGtkRuntimeChrome *runtime)
{
    char text[192];

    if (runtime == NULL) return;

    if (runtime->status_labels[UMI_STUDIO_STATUS_PROBLEMS] != NULL) {
        (void)snprintf(
            text,
            sizeof(text),
            "Problems %zu",
            runtime->view_model_snapshot.retained_diagnostics);
        gtk_label_set_text(
            GTK_LABEL(runtime->status_labels[UMI_STUDIO_STATUS_PROBLEMS]),
            text);
    }

    if (runtime->status_labels[UMI_STUDIO_STATUS_BRANCH] != NULL) {
        (void)snprintf(
            text,
            sizeof(text),
            "Branch %s  %zu change%s",
            runtime->source_control_snapshot.branch[0] != '\0'
                ? runtime->source_control_snapshot.branch
                : "no branch",
            runtime->source_control_snapshot.changes,
            runtime->source_control_snapshot.changes == 1U ? "" : "s");
        gtk_label_set_text(
            GTK_LABEL(runtime->status_labels[UMI_STUDIO_STATUS_BRANCH]),
            text);
    }

    if (runtime->status_labels[UMI_STUDIO_STATUS_TESTS] != NULL) {
        (void)snprintf(
            text,
            sizeof(text),
            "Tests %zu/%zu%s",
            runtime->test_snapshot.passed,
            runtime->test_snapshot.failed,
            runtime->test_snapshot.operation_running ? " running" : "");
        gtk_label_set_text(
            GTK_LABEL(runtime->status_labels[UMI_STUDIO_STATUS_TESTS]),
            text);
    }

    if (runtime->status_labels[UMI_STUDIO_STATUS_AI] != NULL &&
        runtime->owner != NULL && runtime->owner->ui != NULL) {
        UmiStudioAiPlatform *ai = umi_studio_services_ai_platform(
            umi_studio_ui_services(runtime->owner->ui));
        (void)snprintf(
            text,
            sizeof(text),
            "AI %s",
            ai != NULL
                ? runtime_safe(umi_studio_ai_platform_default_provider(ai))
                : "unavailable");
        gtk_label_set_text(
            GTK_LABEL(runtime->status_labels[UMI_STUDIO_STATUS_AI]),
            text);
    }

    if (runtime->status_labels[UMI_STUDIO_STATUS_DEBUG] != NULL) {
        (void)snprintf(
            text,
            sizeof(text),
            "Debug %s",
            runtime->debugger_snapshot.controller_state[0] != '\0'
                ? runtime->debugger_snapshot.controller_state
                : "idle");
        gtk_label_set_text(
            GTK_LABEL(runtime->status_labels[UMI_STUDIO_STATUS_DEBUG]),
            text);
    }
}

static UmiStatus runtime_update_status(
    void *context,
    const UmiStudioRuntimeStatusModel *status)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)context;
    size_t index;

    if (runtime == NULL || status == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    for (index = 0U; index < status->item_count; ++index) {
        const UmiStudioRuntimeStatusItem *item = &status->items[index];
        GtkWidget *label;

        if (item->kind < UMI_STUDIO_STATUS_WORKSPACE ||
            item->kind > UMI_STUDIO_STATUS_CURSOR) {
            continue;
        }

        label = runtime->status_labels[item->kind];
        if (label == NULL) continue;

        gtk_label_set_text(GTK_LABEL(label), item->text);

        if (item->important) {
            gtk_widget_add_css_class(label, "warning");
        } else {
            gtk_widget_remove_css_class(label, "warning");
        }
    }

    runtime_status_override_legacy_services(runtime);
    return UMI_STATUS_OK;
}
