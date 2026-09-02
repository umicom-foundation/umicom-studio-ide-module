/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/resilience.c
 *
 * PURPOSE:
 *   Adapt Framework supervision, circuit and rate-limit primitives to Studio
 *   operational reporting and bounded recovery decisions.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/resilience.h"

#include <string.h>

#include "umicom/studio/operations.h"

/*
 * Provide the studio resilience report operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_resilience_report(UmiStudioServices *services,
                                       UmiStudioResilienceReport *out_report)
{
    UmiStudioOperations *operations;
    UmiResilienceSupervisor *supervisor;
    size_t count;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || out_report == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    operations = umi_studio_services_operations(services);
    supervisor = umi_studio_operations_resilience(operations);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (supervisor == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }
    (void)memset(out_report, 0, sizeof(*out_report));
    count = umi_resilience_supervisor_count(supervisor);
    out_report->supervised_components = count;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < count; ++index) {
        UmiSupervisedSnapshot snapshot;
        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_resilience_supervisor_at(supervisor,
                                         index,
                                         &snapshot) != UMI_STATUS_OK) {
            continue;
        }
        /* Apply this branch only when its contract condition is satisfied. */
        if (snapshot.state == UMI_SUPERVISED_RUNNING) {
            ++out_report->running_components;
        } else /* Apply this branch only when its contract condition is satisfied. */ if (snapshot.state == UMI_SUPERVISED_DEGRADED) {
            ++out_report->degraded_components;
        } else /* Preserve the original failure result so the caller can respond to the correct cause. */ if (snapshot.state == UMI_SUPERVISED_FAILED) {
            ++out_report->failed_components;
        }
    }
    out_report->plugin_circuit = umi_circuit_breaker_snapshot(
        umi_studio_operations_plugin_circuit(operations));
    out_report->tool_rate_limit = umi_rate_limiter_snapshot(
        umi_studio_operations_tool_rate_limiter(operations));
    return UMI_STATUS_OK;
}

/*
 * Provide the studio resilience failure operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_resilience_failure(UmiStudioServices *services,
                                        const char *component_id,
                                        UmiStatus failure,
                                        UmiRestartDecision *out_decision)
{
    UmiStudioOperations *operations;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || component_id == NULL || out_decision == NULL) {
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
    return umi_resilience_supervisor_failure(
        umi_studio_operations_resilience(operations),
        component_id,
        failure,
        umi_studio_operations_now(operations),
        out_decision);
}

/*
 * Provide the studio resilience success operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_resilience_success(UmiStudioServices *services,
                                        const char *component_id)
{
    UmiStudioOperations *operations;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || component_id == NULL) {
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
    return umi_resilience_supervisor_success(
        umi_studio_operations_resilience(operations),
        component_id,
        umi_studio_operations_now(operations));
}

/*
 * Provide the studio resilience allow tool operation used by this module and its client
 * applications.
 */
int umi_studio_resilience_allow_tool(UmiStudioServices *services,
                                     double tokens)
{
    UmiStudioOperations *operations;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) {
        return 0;
    }
    operations = umi_studio_services_operations(services);
    return operations != NULL
        ? umi_rate_limiter_allow(
              umi_studio_operations_tool_rate_limiter(operations),
              tokens,
              umi_studio_operations_now(operations))
        : 0;
}
