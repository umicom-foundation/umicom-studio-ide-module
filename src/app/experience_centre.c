/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: src/app/experience_centre.c
 *
 * PURPOSE:
 *   Provide Studio with a thin product-readiness centre over Framework-owned application experience definitions.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/experience_centre.h"

#include <string.h>

#include "umicom/application/experience_catalogue.h"
#include "umicom/application/experience_plan.h"

const UmiApplicationExperienceDefinition *umi_studio_experience(void)
{
    return umi_application_experience_catalogue_find("org.umicom.studio");
}

UmiStatus umi_studio_experience_status(
    UmiApplicationExperienceStatus *out_status)
{
    const UmiApplicationExperienceDefinition *definition =
        umi_studio_experience();
    if (definition == NULL) return UMI_STATUS_NOT_FOUND;
    return umi_application_experience_status(definition, out_status);
}

size_t umi_studio_experience_centre_count(void)
{
    return umi_application_experience_catalogue_count();
}

UmiStatus umi_studio_experience_centre_row(
    size_t index,
    UmiStudioExperienceRow *out_row)
{
    UmiApplicationExperienceStatus status;
    const UmiApplicationExperienceDefinition *definition;

    if (out_row == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    definition = umi_application_experience_catalogue_at(index);
    if (definition == NULL) return UMI_STATUS_NOT_FOUND;
    if (umi_application_experience_status(definition, &status) != UMI_STATUS_OK)
        return UMI_STATUS_INVALID_STATE;

    memset(out_row, 0, sizeof(*out_row));
    out_row->application_id = definition->application_id;
    out_row->display_name = definition->display_name;
    out_row->default_layout_id = definition->default_layout_id;
    out_row->panel_count = status.panel_count;
    out_row->layout_count = status.layout_count;
    out_row->feature_count = status.feature_count;
    out_row->readiness_percent = status.readiness_percent;
    out_row->has_unverified_p0 =
        umi_application_experience_has_unverified_critical_feature(definition);
    return UMI_STATUS_OK;
}

const UmiExperienceFeatureDefinition *umi_studio_experience_next_feature(void)
{
    return umi_application_experience_next_feature(umi_studio_experience());
}
