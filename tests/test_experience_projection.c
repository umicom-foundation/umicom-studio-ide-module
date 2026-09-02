/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: tests/test_experience_projection.c
 *
 * PURPOSE:
 *   Verify deterministic readiness text projections for Studio UI surfaces.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include <assert.h>
#include <string.h>

#include "umicom/studio/experience_centre.h"
#include "umicom/studio/experience_projection.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioExperienceRow row;
    char buffer[512];
    assert(umi_studio_experience_centre_row(0U, &row) == UMI_STATUS_OK);
    assert(umi_studio_experience_format_row(
        &row, buffer, sizeof(buffer)) == UMI_STATUS_OK);
    assert(strlen(buffer) > 0U);
    return 0;
}
