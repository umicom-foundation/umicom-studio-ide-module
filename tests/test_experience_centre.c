/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: tests/test_experience_centre.c
 *
 * PURPOSE:
 *   Verify Studio exposes the Framework-owned product experience catalogue without duplication.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include <assert.h>

#include "umicom/studio/experience_centre.h"

int main(void)
{
    UmiApplicationExperienceStatus status;
    UmiStudioExperienceRow row;
    assert(umi_studio_experience() != NULL);
    assert(umi_studio_experience_status(&status) == UMI_STATUS_OK);
    assert(umi_studio_experience_centre_count() >= 20U);
    assert(umi_studio_experience_centre_row(0U, &row) == UMI_STATUS_OK);
    assert(row.application_id != NULL);
    return 0;
}
