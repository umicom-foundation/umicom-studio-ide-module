/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/production_readiness.c
 *
 * PURPOSE:
 *   Project the Framework-owned application production control plane into this
 *   thin product without duplicating layout, readiness or acceptance logic.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/production_readiness.h"
#include <string.h>
UmiStatus umi_studio_production_readiness_build(
    const UmiApplicationProductionRuntime *runtime,
    UmiStudioProductionReadiness *out_readiness)
{
    size_t index;
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
    for (index = 0U; index < runtime->features.count; ++index) {
        const UmiApplicationProductionFeatureBinding *feature =
            &runtime->features.entries[index];
        if (!feature->open) continue;
        if (feature->feature->owner == UMI_EXPERIENCE_OWNER_FRAMEWORK)
            out_readiness->open_framework_feature_count += 1U;
        else if (feature->feature->owner == UMI_EXPERIENCE_OWNER_APPLICATION)
            out_readiness->open_application_feature_count += 1U;
        else
            out_readiness->open_adapter_feature_count += 1U;
    }
    return UMI_STATUS_OK;
}

