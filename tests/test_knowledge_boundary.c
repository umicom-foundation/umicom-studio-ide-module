/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Tests
 * File: applications/studio/tests/test_knowledge_boundary.c
 * PURPOSE: Verify retrieval infrastructure does not replace AuthorEngine.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include "umicom/studio/ai_platform.h"

int main(void)
{
    UmiStudioAiPlatformConfig config = umi_studio_ai_platform_config_default();
    UmiStudioAiPlatform *platform = NULL;
    config.knowledge_source_capacity = 8U;
    config.knowledge_vector_capacity = 32U;
    assert(umi_studio_ai_platform_create_configured(&config, &platform)
           == UMI_STATUS_OK);
    assert(umi_studio_ai_platform_authorengine(platform) != NULL);
    assert(umi_studio_ai_platform_knowledge(platform) != NULL);
    assert((void *)umi_studio_ai_platform_authorengine(platform) !=
           (void *)umi_studio_ai_platform_knowledge(platform));
    umi_studio_ai_platform_destroy(platform);
    return 0;
}
