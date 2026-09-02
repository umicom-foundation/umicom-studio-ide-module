/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_test_readiness_contribution.c
 * PURPOSE: Verify Studio remains a thin Framework test-readiness placement.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>

#include "umicom/studio/test_readiness_contribution.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    const UmiStudioTestReadinessContribution *item =
        umi_studio_test_readiness_contribution();
    assert(item != NULL);
    assert(strcmp(item->product_id, "studio") == 0);
    assert(strcmp(item->profile->preset, "windows-ucrt64-debug") == 0);
    assert(item->command_count ==
        umi_test_platform_build_readiness_command_count());
    return 0;
}
