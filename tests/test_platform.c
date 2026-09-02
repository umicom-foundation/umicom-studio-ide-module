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
#include "umicom/studio/platform.h"

#include <stdlib.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioServices *services = NULL;
    UmiStudioPlatformReport report;

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_studio_services_create(NULL, NULL, &services) != UMI_STATUS_OK)
        return EXIT_FAILURE;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_studio_platform_check(services, 0, 0, &report) != UMI_STATUS_OK) {
        umi_studio_services_destroy(services);
        return EXIT_FAILURE;
    }
    /* Apply this operation only while the related capability or state is available. */
    if (!report.environment_ready ||
        !report.discovery.compile_probe_passed ||
        !report.discovery.link_probe_passed ||
        !report.discovery.runtime_probe_passed) {
        umi_studio_services_destroy(services);
        return EXIT_FAILURE;
    }
    umi_studio_platform_report_dispose(&report);
    umi_studio_services_destroy(services);
    return EXIT_SUCCESS;
}
