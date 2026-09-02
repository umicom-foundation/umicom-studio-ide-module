/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_version.c
 *
 * PURPOSE:
 *   Verify the public Studio version structure and string remain synchronised
 *   with the current public Studio release.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/version.h"

#include <assert.h>
#include <string.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiVersion version = umi_studio_version();
    assert(version.major == 0U);
    assert(version.minor == 23U);
    assert(version.patch == 0U);
    assert(strcmp(umi_studio_version_string(), "0.23.0") == 0);
    return 0;
}
