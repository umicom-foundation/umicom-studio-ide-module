/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/authorengine.c
 *
 * PURPOSE:
 *   Create validated AuthorEngine integration configuration for Studio.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Only configuration crosses this boundary; the AuthorEngine process remains independently buildable and replaceable.
 */

#include "umicom/studio/authorengine.h"

#include <stdio.h>
#include <string.h>

/*
 * Provide the studio authorengine config operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_authorengine_config(const char *executable,
                                         const char *workspace,
                                         UmiAiAuthorEngineConfig *config)
{
    int a;
    int b;
    int c;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (executable == NULL || workspace == NULL || config == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    (void)memset(config, 0, sizeof(*config));
    a = snprintf(config->executable, sizeof(config->executable), "%s", executable);
    b = snprintf(config->workspace, sizeof(config->workspace), "%s", workspace);
    c = snprintf(config->provider, sizeof(config->provider), "%s", "studio");
    /* Apply this branch only when its contract condition is satisfied. */
    if (a < 0 || b < 0 || c < 0 ||
        (size_t)a >= sizeof(config->executable) ||
        (size_t)b >= sizeof(config->workspace) ||
        (size_t)c >= sizeof(config->provider)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    return umi_ai_authorengine_validate(config);
}
