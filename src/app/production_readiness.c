/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/production_readiness.c
 *
 * PURPOSE:
 *   Project the Framework-owned application production control plane into this
 *   thin product without duplicating layout, readiness or acceptance logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/production_readiness.h"
#include <string.h>
/*
 * Provide the studio production readiness build operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_production_readiness_build(
    const UmiApplicationProductionRuntime *runtime,
    UmiStudioProductionReadiness *out_readiness)
{
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || out_readiness == NULL || !runtime->initialised ||
        strcmp(runtime->binding.experience->application_id,
               "org.umicom.studio") != 0)
        return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_readiness, 0, sizeof(*out_readiness));
    out_readiness->state = runtime->acceptance.state;
    out_readiness->panel_count = runtime->panels.count;
    out_readiness->layout_count = runtime->layouts.count;
    out_readiness->readiness_percent = runtime->readiness.combined_percent;
    out_readiness->launchable = runtime->acceptance.accepted;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < runtime->features.count; ++index) {
        const UmiApplicationProductionFeatureBinding *feature =
            &runtime->features.entries[index];
        /* Apply this branch only when its contract condition is satisfied. */
        if (!feature->open) continue;
        /* Apply this branch only when its contract condition is satisfied. */
        if (feature->feature->owner == UMI_EXPERIENCE_OWNER_FRAMEWORK)
            out_readiness->open_framework_feature_count += 1U;
        else /* Apply this branch only when its contract condition is satisfied. */ if (feature->feature->owner == UMI_EXPERIENCE_OWNER_APPLICATION)
            out_readiness->open_application_feature_count += 1U;
        /* Use this fallback path when the earlier condition does not apply. */
        else
            out_readiness->open_adapter_feature_count += 1U;
    }
    return UMI_STATUS_OK;
}

