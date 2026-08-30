/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: src/app/application_feature_centre.c
 *
 * PURPOSE:
 *   Project Framework-owned feature backlog and ownership into Studio implementation views.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/application_feature_centre.h"

#include "umicom/application/experience_catalogue.h"

size_t umi_studio_application_feature_backlog_count(const char *application_id)
{
    const UmiApplicationExperienceDefinition *experience =
        umi_application_experience_catalogue_find(application_id);
    return umi_application_ui_feature_backlog_count(experience);
}

UmiStatus umi_studio_application_feature_backlog_row(
    const char *application_id,
    size_t index,
    UmiApplicationFeatureProjection *out_row)
{
    const UmiApplicationExperienceDefinition *experience =
        umi_application_experience_catalogue_find(application_id);
    return experience != NULL
        ? umi_application_ui_feature_backlog_row(experience, index, out_row)
        : UMI_STATUS_NOT_FOUND;
}

UmiStatus umi_studio_application_ownership_rows(
    const char *application_id,
    UmiApplicationUiOwnershipRow rows[UMI_APPLICATION_UI_OWNERSHIP_ROW_COUNT])
{
    const UmiApplicationExperienceDefinition *experience =
        umi_application_experience_catalogue_find(application_id);
    return experience != NULL
        ? umi_application_ui_ownership_rows(experience, rows)
        : UMI_STATUS_NOT_FOUND;
}
