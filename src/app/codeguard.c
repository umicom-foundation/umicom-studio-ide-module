/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/codeguard.c
 *
 * PURPOSE:
 *   Integrate Framework CodeGuard analysis into Studio security and
 *   consolidation workflows.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* BEGINNER NOTE:
 * This file is intentionally small and focused. CodeGuard separates scanning
 * rules, analysis engines and reports so new developers can understand one
 * responsibility at a time and test it independently.
 */
#include "umicom/studio/codeguard.h"

#include <stdlib.h>

/* Keep Studio state minimal: Framework owns all scanning rules and result models. */
struct UmiStudioCodeGuard {
    UmiCodeGuardService *service;
};

/* Allocate the thin facade, then initialise its reusable Framework service. */
UmiStatus umi_studio_codeguard_create(
    const char *root,
    UmiStudioCodeGuard **out_guard)
{
    UmiStudioCodeGuard *guard;
    UmiStatus status;
    /* A workspace and output location are required to establish clear ownership. */
    if (root == NULL || root[0] == '\0' || out_guard == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_guard = NULL;
    guard = (UmiStudioCodeGuard *)calloc(1U, sizeof(*guard));
    /* Allocation failure leaves the caller's output predictably NULL. */
    if (guard == NULL) {
        return UMI_STATUS_OUT_OF_MEMORY;
    }
    status = umi_codeguard_service_create(root, &guard->service);
    /* Release the facade when Framework could not create the owned service. */
    if (status != UMI_STATUS_OK) {
        free(guard);
        return status;
    }
    *out_guard = guard;
    return UMI_STATUS_OK;
}

/* Release resources in the reverse order in which create acquired them. */
void umi_studio_codeguard_destroy(UmiStudioCodeGuard *guard)
{
    /* NULL-safe destruction simplifies cleanup after partially completed startup. */
    if (guard != NULL) {
        umi_codeguard_service_destroy(guard->service);
        free(guard);
    }
}

/* Delegate scanning directly to Framework without copying quality rules into Studio. */
UmiStatus umi_studio_codeguard_scan(
    UmiStudioCodeGuard *guard,
    const UmiCodeGuardProfile *profile)
{
    /* Both the facade and caller-selected profile are required for a scan. */
    if (guard == NULL || profile == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return umi_codeguard_service_scan(guard->service, profile);
}

/* Borrow Framework's latest detailed result for immediate Studio rendering. */
const UmiCodeGuardResult *umi_studio_codeguard_result(
    const UmiStudioCodeGuard *guard)
{
    /* An absent facade has no result and is represented by NULL. */
    if (guard == NULL) {
        return NULL;
    }
    return umi_codeguard_service_result(guard->service);
}

/* Copy the lightweight summary so UI code cannot mutate Framework scan state. */
UmiCodeGuardSummary umi_studio_codeguard_summary(const UmiStudioCodeGuard *guard)
{
    /* A zero summary lets an empty Quality Centre render without unsafe pointers. */
    if (guard == NULL) {
        return (UmiCodeGuardSummary){0};
    }
    return umi_codeguard_service_summary(guard->service);
}

/* Project Framework documentation evidence into Studio's active quality session. */
UmiStatus umi_studio_codeguard_review_documentation(
    const UmiStudioCodeGuard *guard,
    const char *path,
    const UmiCodeGuardDocumentationPolicy *policy,
    UmiCodeGuardDocumentationReport *out_report)
{
    /* Requiring a live facade prevents actions after the Quality Centre is closed. */
    if (guard == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return umi_codeguard_documentation_scan_file(path, policy, out_report);
}
