/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_history.c
 *
 * PURPOSE:
 *   Verify ordered Studio operational-history persistence and reconstruction.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* These assertions perform setup as well as checks, including in release builds. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "studio_test_state.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/history.h"

#include <assert.h>
#include <string.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    /* Isolate default preferences and data before starting shared services. */
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;

    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioServices *services;
    UmiStudioHistoryRecord record;
    assert(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    services = umi_studio_bootstrap_services(bootstrap);
    assert(umi_studio_history_append(services, "build", UMI_STATUS_OK,
                                     "Build completed", 55U, NULL) == UMI_STATUS_OK);
    assert(umi_studio_history_count(services) == 1U);
    assert(umi_studio_history_read(services, 0U, &record) == UMI_STATUS_OK);
    assert(strcmp(record.category, "build") == 0);
    assert(strcmp(record.message, "Build completed") == 0);
    assert(record.correlation_id == 55U);
    umi_studio_bootstrap_destroy(bootstrap);
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
