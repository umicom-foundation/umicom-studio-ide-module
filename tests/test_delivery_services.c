/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_delivery_services.c
 *
 * PURPOSE:
 *   Verify one Studio integration path over the Framework delivery platform.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * The test keeps product integration small so failures are easy for a new developer to locate.
 */

/* These assertions perform setup as well as checks, including in release builds. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "studio_test_state.h"
#include <assert.h>
#include "umicom/studio/services.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void) {
    /* Isolate default preferences and data before starting shared services. */
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;

    UmiStudioServices *services = NULL;
    assert(umi_studio_services_create(NULL, NULL, &services) == UMI_STATUS_OK);
    assert(umi_studio_services_delivery_platform(services) != NULL);
    umi_studio_services_destroy(services);
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
