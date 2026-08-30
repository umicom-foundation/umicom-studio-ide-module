/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Tests
 * File: applications/studio/tests/test_vibe_coding.c
 *
 * PURPOSE:
 *   Verify a plain-language goal becomes a repository-aware, review-gated
 *   coding session without retaining a raw API key.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>

#include "umicom/studio/vibe_coding.h"

int main(void)
{
    UmiStudioAiPlatform *platform = NULL;
    UmiStudioVibeCodingSession session;

    assert(umi_studio_ai_platform_create(&platform) == UMI_STATUS_OK);
    assert(umi_studio_vibe_coding_prepare(
        platform, "vibe.1",
        "Add a beginner panel that explains the active compiler error.",
        "applications/studio/src/app/learning_centre.c", &session) ==
        UMI_STATUS_OK);
    assert(session.state == UMI_STUDIO_VIBE_CODING_CONTEXT_READY);
    assert(session.rag_enabled == 1);
    assert(session.stream_responses == 1);
    assert(session.requires_patch_approval == 1);
    assert(session.plan.repository_context.file_count > 0U);
    assert(session.secret_reference[0] == '\0');
    assert(strcmp(umi_studio_vibe_coding_state_text(session.state),
                  "context ready") == 0);
    umi_studio_ai_platform_destroy(platform);
    return 0;
}
