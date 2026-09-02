/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: src/app/application_runtime_centre.c
 *
 * PURPOSE:
 *   Project the Framework runtime catalogue into Studio-facing application summary rows.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/application_runtime_centre.h"

#include "umicom/application/experience_catalogue.h"

/*
 * Return the number of records represented by studio application runtime centre without
 * changing their state.
 */
size_t umi_studio_application_runtime_centre_count(void)
{
    return umi_application_experience_catalogue_count();
}

/*
 * Provide the studio application runtime centre row operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_runtime_centre_row(
    size_t index,
    UmiApplicationUiSummaryRow *out_row)
{
    const UmiApplicationExperienceDefinition *experience =
        umi_application_experience_catalogue_at(index);
    return experience != NULL
        ? umi_application_ui_summary_row(experience, out_row)
        : UMI_STATUS_NOT_FOUND;
}

/*
 * Provide the studio application runtime catalogue status operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_application_runtime_catalogue_status(
    UmiApplicationRuntimeCatalogueStatus *out_status)
{
    return umi_application_runtime_catalogue_status(out_status);
}
