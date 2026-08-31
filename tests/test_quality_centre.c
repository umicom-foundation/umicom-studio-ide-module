/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_quality_centre.c
 *
 * PURPOSE:
 *   Verify the test quality centre behavior for
 *   Umicom Studio IDE.
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

#include "quality_centre_fixture.h"

/* Verify view selection is copied into a stable value snapshot. */
int main(void)
{
    UmiStudioQualityCentre *centre = umi_test_quality_centre();
    UmiStudioQualitySnapshot snapshot;
    /* Selection advances the presentation revision from one to two. */
    assert(umi_studio_quality_centre_activate(
        centre, "evidence") == UMI_STATUS_OK);
    assert(umi_studio_quality_centre_snapshot(
        centre, &snapshot) == UMI_STATUS_OK);
    assert(strcmp(snapshot.active_view, "evidence") == 0);
    assert(snapshot.evidence == 0U);
    assert(snapshot.documentation_files == 0U);
    assert(snapshot.revision == 2U);
    umi_studio_quality_centre_destroy(centre);
    return 0;
}
