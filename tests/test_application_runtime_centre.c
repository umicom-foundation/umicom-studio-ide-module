/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: tests/test_application_runtime_centre.c
 *
 * PURPOSE:
 *   Verify Studio projects the Framework runtime catalogue without owning it.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

#include <assert.h>

#include "umicom/studio/application_runtime_centre.h"

int main(void)
{
    UmiApplicationUiSummaryRow row;
    UmiApplicationRuntimeCatalogueStatus status;
    assert(umi_studio_application_runtime_centre_count() > 0U);
    assert(umi_studio_application_runtime_centre_row(0U, &row) == UMI_STATUS_OK);
    assert(umi_studio_application_runtime_catalogue_status(&status) == UMI_STATUS_OK);
    return 0;
}
