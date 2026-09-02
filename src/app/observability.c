/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/observability.c
 *
 * PURPOSE:
 *   Record and summarise Studio operational evidence using reusable Framework
 *   observability services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/observability.h"

#include <stdio.h>
#include <string.h>

#include "umicom/studio/operations.h"

/*
 * Provide the studio observability report operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_observability_report(
    UmiStudioServices *services,
    UmiStudioObservabilityReport *out_report)
{
    UmiStudioOperations *operations;
    UmiAuditLog *audit;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || out_report == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    operations = umi_studio_services_operations(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (operations == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }
    (void)memset(out_report, 0, sizeof(*out_report));
    status = umi_observability_snapshot_capture(
        umi_studio_operations_metrics(operations),
        umi_studio_operations_traces(operations),
        umi_studio_operations_audit(operations),
        umi_studio_operations_readiness(operations),
        umi_studio_operations_events(operations),
        &out_report->snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    audit = umi_studio_operations_audit(operations);
    out_report->successful_audits = umi_audit_log_outcome_count(
        audit,
        UMI_AUDIT_SUCCEEDED);
    out_report->denied_audits = umi_audit_log_outcome_count(
        audit,
        UMI_AUDIT_DENIED);
    out_report->failed_audits = umi_audit_log_outcome_count(
        audit,
        UMI_AUDIT_FAILED);
    out_report->profile_samples = umi_profiler_count(
        umi_studio_operations_profiler(operations));
    return UMI_STATUS_OK;
}

/*
 * Provide the studio observability event operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_observability_event(UmiStudioServices *services,
                                         const char *category,
                                         const char *message,
                                         UmiDiagnosticSeverity severity,
                                         uint64_t correlation_id)
{
    UmiStudioOperations *operations;
    UmiOperationalEvent event = {0};
    UmiStatus status;
    uint64_t now;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || category == NULL || message == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    operations = umi_studio_services_operations(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (operations == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }
    now = umi_studio_operations_now(operations);
    event.timestamp_ns = now;
    event.correlation_id = correlation_id;
    event.severity = severity;
    (void)snprintf(event.category, sizeof(event.category), "%s", category);
    (void)snprintf(event.message, sizeof(event.message), "%s", message);
    status = umi_operational_event_log_append(
        umi_studio_operations_events(operations),
        &event);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_metrics_counter_add(
            umi_studio_operations_metrics(operations),
            "studio.operations.events",
            1.0,
            now);
    }
    return status;
}

/*
 * Provide the studio observability begin operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_observability_begin(UmiStudioServices *services,
                                         const char *name,
                                         uint64_t trace_id,
                                         uint64_t parent_span_id,
                                         UmiProfileScope *out_scope,
                                         uint64_t *out_span_id)
{
    UmiStudioOperations *operations;
    UmiStatus status;
    uint64_t now;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || name == NULL || out_scope == NULL ||
        out_span_id == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    operations = umi_studio_services_operations(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (operations == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }
    now = umi_studio_operations_now(operations);
    status = umi_profiler_begin(name, now, out_scope);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    return umi_trace_span_begin(umi_studio_operations_traces(operations),
                                trace_id,
                                parent_span_id,
                                name,
                                now,
                                out_span_id);
}

/*
 * Provide the studio observability end operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_observability_end(UmiStudioServices *services,
                                       const UmiProfileScope *scope,
                                       uint64_t span_id,
                                       UmiStatus operation_status)
{
    UmiStudioOperations *operations;
    UmiStatus status;
    uint64_t now;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || scope == NULL || span_id == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    operations = umi_studio_services_operations(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (operations == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }
    now = umi_studio_operations_now(operations);
    status = umi_profiler_end(umi_studio_operations_profiler(operations),
                              scope,
                              now);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    return umi_trace_span_end(
        umi_studio_operations_traces(operations),
        span_id,
        operation_status == UMI_STATUS_OK ? UMI_TRACE_OK : UMI_TRACE_ERROR,
        now);
}
