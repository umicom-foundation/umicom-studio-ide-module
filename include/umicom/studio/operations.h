/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/operations.h
 *
 * PURPOSE:
 *   Own Studio's shared Framework security, plug-in, observability and
 *   resilience services without duplicating their implementations in the
 *   product layer.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_OPERATIONS_H
#define UMICOM_STUDIO_OPERATIONS_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio operations data shared with callers of this public contract.
 */
typedef struct UmiStudioOperations UmiStudioOperations;

/**
 * Represent the studio operations report data shared with callers of this public contract.
 */
typedef struct UmiStudioOperationsReport {
    size_t identities;
    size_t roles;
    size_t sessions;
    size_t trusted_workspaces;
    size_t security_events;
    size_t denied_security_events;
    size_t plugins;
    size_t plugin_contributions;
    size_t metrics;
    size_t trace_spans;
    size_t audit_records;
    size_t readiness_checks;
    size_t operational_events;
    size_t supervised_components;
    int ready;
} UmiStudioOperationsReport;

/**
 * Initialise studio operations from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_operations_create(UmiClock *clock,
                                       UmiStudioOperations **out_operations);
/**
 * Release or reset state held by studio operations so the same storage can be reused
 * safely.
 */
void umi_studio_operations_destroy(UmiStudioOperations *operations);

/**
 * Provide the studio operations security operation used by this module and its client
 * applications.
 */
UmiSecurityContext *umi_studio_operations_security(UmiStudioOperations *operations);
/**
 * Provide the studio operations plugins operation used by this module and its client
 * applications.
 */
UmiPluginHost *umi_studio_operations_plugins(UmiStudioOperations *operations);
/**
 * Provide the studio operations metrics operation used by this module and its client
 * applications.
 */
UmiMetricsRegistry *umi_studio_operations_metrics(UmiStudioOperations *operations);
/**
 * Provide the studio operations traces operation used by this module and its client
 * applications.
 */
UmiTraceStore *umi_studio_operations_traces(UmiStudioOperations *operations);
/**
 * Provide the studio operations audit operation used by this module and its client
 * applications.
 */
UmiAuditLog *umi_studio_operations_audit(UmiStudioOperations *operations);
/**
 * Provide the studio operations profiler operation used by this module and its client
 * applications.
 */
UmiProfiler *umi_studio_operations_profiler(UmiStudioOperations *operations);
/**
 * Provide the studio operations readiness operation used by this module and its client
 * applications.
 */
UmiReadinessRegistry *umi_studio_operations_readiness(UmiStudioOperations *operations);
/**
 * Provide the studio operations events operation used by this module and its client
 * applications.
 */
UmiOperationalEventLog *umi_studio_operations_events(UmiStudioOperations *operations);
/**
 * Provide the studio operations resilience operation used by this module and its client
 * applications.
 */
UmiResilienceSupervisor *umi_studio_operations_resilience(UmiStudioOperations *operations);
/**
 * Provide the studio operations health gate operation used by this module and its client
 * applications.
 */
UmiHealthGate *umi_studio_operations_health_gate(UmiStudioOperations *operations);
/**
 * Provide the studio operations plugin circuit operation used by this module and its
 * client applications.
 */
UmiCircuitBreaker *umi_studio_operations_plugin_circuit(UmiStudioOperations *operations);
/**
 * Provide the studio operations tool rate limiter operation used by this module and its
 * client applications.
 */
UmiRateLimiter *umi_studio_operations_tool_rate_limiter(UmiStudioOperations *operations);
/**
 * Provide the studio operations now operation used by this module and its client
 * applications.
 */
uint64_t umi_studio_operations_now(UmiStudioOperations *operations);

/**
 * Provide the studio operations report operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_operations_report(const UmiStudioOperations *operations,
                                       UmiStudioOperationsReport *out_report);

#ifdef __cplusplus
}
#endif

#endif
