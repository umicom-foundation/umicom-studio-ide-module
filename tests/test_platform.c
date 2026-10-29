/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_platform.c
 *
 * PURPOSE:
 *   Verify that Studio consumes the Framework toolchain and environment
 *   services without owning a duplicate discovery implementation.
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
#include "umicom/studio/platform.h"

#include <stdlib.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    /* Isolate default preferences and data before starting shared services. */
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;

    UmiStudioServices *services = NULL;
    UmiStudioPlatformReport report;

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_studio_services_create(NULL, NULL, &services) != UMI_STATUS_OK) {
        (void)StudioTestStateEnd(&test_state);
        return EXIT_FAILURE;
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_studio_platform_check(services, 0, 0, &report) != UMI_STATUS_OK) {
        umi_studio_services_destroy(services);
        (void)StudioTestStateEnd(&test_state);
        return EXIT_FAILURE;
    }
    /* Apply this operation only while the related capability or state is available. */
    if (!report.environment_ready ||
        !report.discovery.compile_probe_passed ||
        !report.discovery.link_probe_passed ||
        !report.discovery.runtime_probe_passed) {
        umi_studio_services_destroy(services);
        (void)StudioTestStateEnd(&test_state);
        return EXIT_FAILURE;
    }
    umi_studio_platform_report_dispose(&report);
    umi_studio_services_destroy(services);
    if (!StudioTestStateEnd(&test_state)) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
