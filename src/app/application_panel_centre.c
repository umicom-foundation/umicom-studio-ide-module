/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: src/app/application_panel_centre.c
 *
 * PURPOSE:
 *   Resolve product panels and layouts through Framework-owned experience projections.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/application_panel_centre.h"

#include "umicom/application/experience_catalogue.h"

UmiStatus umi_studio_application_panel_row(
    const char *application_id,
    size_t index,
    UmiApplicationPanelProjection *out_row)
{
    const UmiApplicationExperienceDefinition *experience =
        umi_application_experience_catalogue_find(application_id);
    return experience != NULL
        ? umi_application_ui_panel_row(experience, index, out_row)
        : UMI_STATUS_NOT_FOUND;
}

UmiStatus umi_studio_application_layout_row(
    const char *application_id,
    size_t index,
    UmiApplicationLayoutProjection *out_row)
{
    const UmiApplicationExperienceDefinition *experience =
        umi_application_experience_catalogue_find(application_id);
    return experience != NULL
        ? umi_application_ui_layout_row(experience, index, out_row)
        : UMI_STATUS_NOT_FOUND;
}
