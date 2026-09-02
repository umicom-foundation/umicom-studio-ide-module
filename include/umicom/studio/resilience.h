/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/resilience.h
 *
 * PURPOSE:
 *   Expose Studio supervision, circuit-breaker and rate-limit state through
 *   shared Framework resilience services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_RESILIENCE_H
#define UMICOM_STUDIO_RESILIENCE_H

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio resilience report data shared with callers of this public contract.
 */
typedef struct UmiStudioResilienceReport {
    size_t supervised_components;
    size_t running_components;
    size_t degraded_components;
    size_t failed_components;
    UmiCircuitBreakerSnapshot plugin_circuit;
    UmiRateLimiterSnapshot tool_rate_limit;
} UmiStudioResilienceReport;

/**
 * Provide the studio resilience report operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_resilience_report(UmiStudioServices *services,
                                       UmiStudioResilienceReport *out_report);
/**
 * Provide the studio resilience failure operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_resilience_failure(UmiStudioServices *services,
                                        const char *component_id,
                                        UmiStatus failure,
                                        UmiRestartDecision *out_decision);
/**
 * Provide the studio resilience success operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_resilience_success(UmiStudioServices *services,
                                        const char *component_id);
/**
 * Provide the studio resilience allow tool operation used by this module and its client
 * applications.
 */
int umi_studio_resilience_allow_tool(UmiStudioServices *services,
                                     double tokens);

#ifdef __cplusplus
}
#endif

#endif
