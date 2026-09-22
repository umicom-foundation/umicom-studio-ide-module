/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/test_discovery.h
 *
 * PURPOSE:
 *   Select Framework-owned background discovery for the existing Studio
 *   command without creating another provider, queue or publication policy.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_TEST_DISCOVERY_H
#define UMICOM_STUDIO_TEST_DISCOVERY_H
#include "umicom/test_platform/ctest.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct UmiStudioTestService UmiStudioTestService;

/** Owner-thread, one-command opt-in. The queue and service must outlive the job.
 * Call before the canonical Discover command, and always Disarm afterwards,
 * including command failure. Without this opt-in the existing API is synchronous.
 * The generation belongs to the authoritative workspace graph, not the GTK timer.
 * The caller must authorise execution of the configured build's CTest scripts.
 */
UmiStatus UmiStudioTestDiscoveryArm(UmiStudioTestService *service,
    UmiTaskQueue *queue, uint64_t workspaceGeneration);
void UmiStudioTestDiscoveryDisarm(UmiStudioTestService *service);
/** Includes completed work that has not yet been polled/published. */
int UmiStudioTestDiscoveryPending(const UmiStudioTestService *service);
/** Non-waiting owner-thread publication. Call from the existing UI refresh loop
 * before reading views. BUSY means pending; NOT_FOUND means no job. A change in
 * trust, workspace generation or any supplied profile field suppresses results.
 * On completion this consumes the job. outPublished is one only for a successful
 * catalogue publication; it does not certify unbounded GUI rendering/selection.
 */
UmiStatus UmiStudioTestDiscoveryPoll(UmiStudioTestService *service,
    const char *sourceRoot, const char *buildRoot, const char *configuration,
    uint64_t workspaceGeneration, int workspaceAllowed, int *outPublished,
    char *diagnostics, size_t capacity);

#ifdef __cplusplus
}
#endif
#endif
