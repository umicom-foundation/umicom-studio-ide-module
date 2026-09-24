/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/developer_operations.inc
 *
 * PURPOSE:
 *   Present Framework developer-operation projections through existing Studio
 *   status items without duplicating Test, Debug, Terminal or VCS engines.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/developer_operations/service.h"

/* Keep attention styling consistent with the established Studio status strip. */
static void runtime_developer_operations_set_attention(
    GtkWidget *label,
    int attention)
{
    if (label == NULL) return;
    if (attention) gtk_widget_add_css_class(label, "warning");
    else gtk_widget_remove_css_class(label, "warning");
}

/*
 * Re-project the existing Studio service snapshots through the Framework
 * Developer Operations catalogue. This adds shared readiness/attention policy
 * without changing the authoritative application services.
 */
static void runtime_developer_operations_update_status(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiDeveloperOperationInput input;
    UmiDeveloperOperationSnapshot snapshot;
    GtkWidget *label;
    char text[192];
    uint64_t completedTests;
    uint64_t totalTests;

    if (runtime == NULL) return;

    completedTests =
        (uint64_t)runtime->test_snapshot.passed +
        (uint64_t)runtime->test_snapshot.failed +
        (uint64_t)runtime->test_snapshot.skipped +
        (uint64_t)runtime->test_snapshot.cancelled +
        (uint64_t)runtime->test_snapshot.timed_out;
    totalTests = runtime->test_snapshot.test_count > 0U
        ? (uint64_t)runtime->test_snapshot.test_count
        : completedTests;

    UmiDeveloperOperationInputInit(&input);
    input.primary = completedTests;
    input.secondary =
        (uint64_t)runtime->test_snapshot.failed +
        (uint64_t)runtime->test_snapshot.timed_out;
    input.total = totalTests;
    input.active = runtime->test_snapshot.operation_running;
    input.blocked = 0;
    input.revision = runtime->test_snapshot.explorer_revision;

    if (UmiDeveloperOperationsServiceEvaluate(
            "coverage.test-result-summary",
            &input,
            &snapshot) == UMI_STATUS_OK) {
        label = runtime->status_labels[UMI_STUDIO_STATUS_TESTS];
        if (label != NULL) {
            (void)snprintf(
                text,
                sizeof(text),
                "Tests %u/%u  %u failed%s",
                (unsigned)runtime_bounded_badge(
                    runtime->test_snapshot.passed),
                (unsigned)runtime_bounded_badge(
                    runtime->test_snapshot.test_count),
                (unsigned)runtime_bounded_badge(
                    runtime->test_snapshot.failed +
                    runtime->test_snapshot.timed_out),
                runtime->test_snapshot.operation_running
                    ? "  running"
                    : "");
            gtk_label_set_text(GTK_LABEL(label), text);
            runtime_developer_operations_set_attention(
                label,
                input.secondary > 0U);
        }
    }

    UmiDeveloperOperationInputInit(&input);
    input.primary = (uint64_t)runtime->debugger_snapshot.thread_count;
    input.secondary =
        (uint64_t)runtime->debugger_snapshot.breakpoint_count +
        (uint64_t)runtime->debugger_snapshot.watch_count;
    input.total =
        (uint64_t)runtime->debugger_snapshot.stack_frame_count;
    input.active =
        runtime->debugger_snapshot.initialized ||
        runtime->debugger_snapshot.session_count > 0U;
    input.blocked = 0;
    input.revision = runtime->debugger_snapshot.workspace.revision;

    if (UmiDeveloperOperationsServiceEvaluate(
            "debug.session",
            &input,
            &snapshot) == UMI_STATUS_OK) {
        label = runtime->status_labels[UMI_STUDIO_STATUS_DEBUG];
        if (label != NULL) {
            (void)snprintf(
                text,
                sizeof(text),
                "Debug %.96s  %u thread%s",
                runtime->debugger_snapshot.controller_state[0] != '\0'
                    ? runtime->debugger_snapshot.controller_state
                    : "idle",
                (unsigned)runtime_bounded_badge(
                    runtime->debugger_snapshot.thread_count),
                runtime->debugger_snapshot.thread_count == 1U
                    ? ""
                    : "s");
            gtk_label_set_text(GTK_LABEL(label), text);
            runtime_developer_operations_set_attention(
                label, snapshot.attention);
        }
    }

    UmiDeveloperOperationInputInit(&input);
    input.primary = (uint64_t)runtime->source_control_snapshot.changes;
    input.secondary = (uint64_t)runtime->source_control_snapshot.conflicts;
    input.total =
        (uint64_t)runtime->source_control_snapshot.changes +
        (uint64_t)runtime->source_control_snapshot.staged;
    input.active = runtime->source_control_snapshot.available;
    input.blocked = runtime->source_control_snapshot.conflicts > 0U;
    input.revision = runtime->source_control_snapshot.revision;

    if (UmiDeveloperOperationsServiceEvaluate(
            "vcs.status",
            &input,
            &snapshot) == UMI_STATUS_OK) {
        label = runtime->status_labels[UMI_STUDIO_STATUS_BRANCH];
        if (label != NULL) {
            (void)snprintf(
                text,
                sizeof(text),
                "Branch %.104s  %u change%s  %u conflict%s",
                runtime->source_control_snapshot.branch[0] != '\0'
                    ? runtime->source_control_snapshot.branch
                    : "no branch",
                (unsigned)runtime_bounded_badge(
                    runtime->source_control_snapshot.changes),
                runtime->source_control_snapshot.changes == 1U ? "" : "s",
                (unsigned)runtime_bounded_badge(
                    runtime->source_control_snapshot.conflicts),
                runtime->source_control_snapshot.conflicts == 1U ? "" : "s");
            gtk_label_set_text(GTK_LABEL(label), text);
            runtime_developer_operations_set_attention(
                label, snapshot.attention);
        }
    }
}
