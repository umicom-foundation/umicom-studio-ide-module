/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/integration_designer_monitoring.c
 *
 * PURPOSE:
 *   Implement the integration designer monitoring behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Integration Designer monitoring | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/integration_designer_monitoring.h"
#include <string.h>

/*
 * Provide the studio integration designer health operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_integration_designer_health(const UmiStudioIntegrationDesignerCentre *centre,UmiStudioIntegrationDesignerHealth *out_health)
{
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || out_health == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_health,0,sizeof(*out_health));
    umi_integration_run_history_metrics(&centre->fabric.runs,&out_health->runs);
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < centre->fabric.connections.count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (centre->fabric.connections.items[index].enabled) out_health->enabled_connections += 1U;
        /* Use this fallback path when the earlier condition does not apply. */
        else out_health->disabled_connections += 1U;
    }
    out_health->healthy = out_health->disabled_connections == 0U && out_health->runs.failed == 0U;
    return UMI_STATUS_OK;
}

/*
 * Perform studio integration designer find through the module contract so client
 * applications do not duplicate its policy.
 */
const UmiIntegrationRunRecord *umi_studio_integration_designer_find_run(const UmiStudioIntegrationDesignerCentre *centre,const char *run_id)
{
    return centre == NULL ? NULL : umi_integration_run_history_find(&centre->fabric.runs,run_id);
}
