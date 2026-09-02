/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: tests/test_application_feature_centre.c
 *
 * PURPOSE:
 *   Verify Studio exposes Framework-owned backlog and ownership evidence.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include <assert.h>

#include "umicom/studio/application_feature_centre.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiApplicationUiOwnershipRow rows[UMI_APPLICATION_UI_OWNERSHIP_ROW_COUNT];
    assert(umi_studio_application_feature_backlog_count("org.umicom.trader") > 0U);
    assert(umi_studio_application_ownership_rows("org.umicom.trader", rows) == UMI_STATUS_OK);
    return 0;
}
