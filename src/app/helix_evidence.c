/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/helix_evidence.c
 *
 * PURPOSE:
 *   Store and retrieve Studio Helix evidence summaries.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * The adapter stores concise evidence rather than model reasoning text, preserving an auditable operational record.
 */

#include "umicom/studio/helix_evidence.h"

/*
 * Provide the studio helix record evidence operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_helix_record_evidence(UmiStudioAiPlatform *platform,
                                           const char *key,
                                           const char *summary)
{
    UmiHelixRuntime *runtime = umi_studio_ai_platform_helix(platform);
    return runtime != NULL
        ? umi_helix_memory_put(&runtime->memory, key, summary)
        : UMI_STATUS_INVALID_STATE;
}

/*
 * Provide the studio helix evidence operation used by this module and its client
 * applications.
 */
const char *umi_studio_helix_evidence(UmiStudioAiPlatform *platform,
                                      const char *key)
{
    UmiHelixRuntime *runtime = umi_studio_ai_platform_helix(platform);
    return runtime != NULL ? umi_helix_memory_get(&runtime->memory, key) : NULL;
}
