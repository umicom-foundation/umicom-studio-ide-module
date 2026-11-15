/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/codeguard.h
 *
 * PURPOSE:
 *   Expose Studio integration for CodeGuard security, duplication and
 *   architecture quality analysis.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is intentionally small and focused. CodeGuard separates scanning
 * rules, analysis engines and reports so new developers can understand one
 * responsibility at a time and test it independently.
 */
#ifndef UMICOM_STUDIO_CODEGUARD_H
#define UMICOM_STUDIO_CODEGUARD_H

#include "umicom/codeguard/codeguard.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Studio owns this small facade; the reusable scanner and evidence stay in Framework. */
typedef struct UmiStudioCodeGuard UmiStudioCodeGuard;

/* Create a Studio quality session for one workspace root. The returned facade
 * is owned by the caller and must be released with the matching destroy call. */
UmiStatus umi_studio_codeguard_create(
    const char *root,
    UmiStudioCodeGuard **out_guard);

/* Release the facade and its Framework service; passing NULL is harmless. */
void umi_studio_codeguard_destroy(UmiStudioCodeGuard *guard);

/* Run the selected Framework quality profile for the facade's workspace. */
UmiStatus umi_studio_codeguard_scan(
    UmiStudioCodeGuard *guard,
    const UmiCodeGuardProfile *profile);

/* Synchronous cooperative scan using Framework's shared token. The facade
 * still requires an explicit profile. Cancellation preserves the prior
 * complete result and its finding borrows. Serialize facade access and keep
 * the token alive until return; only request may run concurrently. This
 * adapter does not create a worker, a scan menu action or a GUI control. */
UmiStatus umi_studio_codeguard_scan_cancellable(UmiStudioCodeGuard *guard,
    const UmiCodeGuardProfile *profile, const UmiCancellationToken *cancellation);

/* Borrow the latest detailed scan result; Studio and callers must not free it. */
const UmiCodeGuardResult *umi_studio_codeguard_result(const UmiStudioCodeGuard *guard);

/* Return a value snapshot suitable for status bars and Quality Centre cards. */
UmiCodeGuardSummary umi_studio_codeguard_summary(const UmiStudioCodeGuard *guard);

/* Review documentation guidance for a selected source file. The facade proves
 * that a Studio quality session is active, while Framework performs the scan. */
UmiStatus umi_studio_codeguard_review_documentation(
    const UmiStudioCodeGuard *guard,
    const char *path,
    const UmiCodeGuardDocumentationPolicy *policy,
    UmiCodeGuardDocumentationReport *out_report);

/* Prepare a copied job without starting it. Unlike the Framework default-profile
 * API, Studio requires an explicit profile. A valid output is cleared on failure.
 * Job lifetime is independent of the facade; scheduling uses Framework queues. */
UmiStatus umi_studio_codeguard_create_job(UmiStudioCodeGuard *guard,
    const UmiCodeGuardProfile *profile, UmiCodeGuardJob **out_job);

/* Adopt a complete same-workspace job on its creating thread. Serialize facade
 * access; failure preserves retained findings. Success invalidates old finding
 * borrows but keeps result object identity. The caller still destroys the job.
 * This facade does not connect a GUI action or choose newest-job ordering. */
UmiStatus umi_studio_codeguard_adopt_job(UmiStudioCodeGuard *guard, UmiCodeGuardJob *job);

#ifdef __cplusplus
}
#endif

#endif
