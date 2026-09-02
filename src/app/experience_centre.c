/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: src/app/experience_centre.c
 *
 * PURPOSE:
 *   Provide Studio with a thin product-readiness centre over Framework-owned application experience definitions.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/experience_centre.h"

#include <string.h>

#include "umicom/application/experience_catalogue.h"
#include "umicom/application/experience_plan.h"

/* Provide the studio experience operation used by this module and its client applications. */
const UmiApplicationExperienceDefinition *umi_studio_experience(void)
{
    return umi_application_experience_catalogue_find("org.umicom.studio");
}

/*
 * Provide the studio experience status operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_experience_status(
    UmiApplicationExperienceStatus *out_status)
{
    const UmiApplicationExperienceDefinition *definition =
        umi_studio_experience();
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (definition == NULL) return UMI_STATUS_NOT_FOUND;
    return umi_application_experience_status(definition, out_status);
}

/*
 * Return the number of records represented by studio experience centre without changing
 * their state.
 */
size_t umi_studio_experience_centre_count(void)
{
    return umi_application_experience_catalogue_count();
}

/*
 * Provide the studio experience centre row operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_experience_centre_row(
    size_t index,
    UmiStudioExperienceRow *out_row)
{
    UmiApplicationExperienceStatus status;
    const UmiApplicationExperienceDefinition *definition;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_row == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    definition = umi_application_experience_catalogue_at(index);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (definition == NULL) return UMI_STATUS_NOT_FOUND;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
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

/*
 * Provide the studio experience next feature operation used by this module and its client
 * applications.
 */
const UmiExperienceFeatureDefinition *umi_studio_experience_next_feature(void)
{
    return umi_application_experience_next_feature(umi_studio_experience());
}
