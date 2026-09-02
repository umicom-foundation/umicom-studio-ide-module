/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_security_views.c
 *
 * PURPOSE:
 *   Verify the test security views behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE Tests | Security Centre views v2 | Sammy Hegab | Umicom Foundation | MIT */
#include <assert.h>
#include <string.h>
#include "security_centre_fixture.h"
#include "umicom/studio/security_views.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioSecurityCentre *centre = umi_test_seeded_security_centre();
    UmiStudioSecurityView identities;
    UmiStudioSecurityView approvals;
    assert(umi_studio_security_view_build(
        centre,10U,UMI_STUDIO_SECURITY_VIEW_IDENTITIES,&identities) == UMI_STATUS_OK);
    assert(strcmp(identities.id,"identities") == 0 && identities.item_count == 1U);
    assert(umi_studio_security_view_build(
        centre,10U,UMI_STUDIO_SECURITY_VIEW_APPROVALS,&approvals) == UMI_STATUS_OK);
    assert(approvals.item_count == 1U && strstr(approvals.summary,"1 items") != NULL);
    umi_studio_security_centre_destroy(centre);
    return 0;
}
