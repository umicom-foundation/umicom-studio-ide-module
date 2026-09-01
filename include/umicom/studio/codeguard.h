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

#ifdef __cplusplus
}
#endif

#endif
