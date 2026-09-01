/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_ai_platform.c
 *
 * PURPOSE:
 *   Validate Studio's Framework-backed AI and Helix integration.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This focused test verifies one product adapter while keeping the provider-neutral implementation inside Framework.
 */



#include <assert.h>

#include "umicom/studio/ai_platform.h"

int main(void)
{
    UmiStudioAiPlatform *platform = NULL;
    UmiAiAuthorEngineServiceSnapshot snapshot;
    UmiStudioAiWorkbenchProfile workbench;
    assert(umi_studio_ai_platform_create(&platform) == UMI_STATUS_OK);
    assert(platform != NULL);
    assert(umi_studio_ai_platform_runtime(platform) != NULL);
    assert(umi_studio_ai_platform_helix(platform) != NULL);
    assert(umi_studio_ai_platform_authorengine(platform) != NULL);
    assert(umi_studio_ai_platform_snapshot(platform, &snapshot)
        == UMI_STATUS_OK);
    assert(snapshot.runtimes == 3U);
    assert(snapshot.context_sources == 3U);
    assert(snapshot.sessions == 1U);
    assert(umi_studio_ai_platform_workbench_profile(platform, &workbench) ==
           UMI_STATUS_OK);
    assert(workbench.rag_enabled == 1);
    assert(workbench.stream_responses == 1);
    assert(workbench.require_patch_approval == 1);
    assert(workbench.remote_configured == 0);
    umi_studio_ai_platform_destroy(platform);
    return 0;
}
