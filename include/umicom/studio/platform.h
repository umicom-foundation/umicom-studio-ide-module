/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/platform.h
 *
 * PURPOSE:
 *   Expose the Studio-facing view of Framework toolchain discovery, dependency
 *   validation, child-process environment construction, repository scaffolding,
 *   and reusable platform capability status without duplicating the Framework
 *   implementation inside the product.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_PLATFORM_H
#define UMICOM_STUDIO_PLATFORM_H

#include "umicom/umicom.h"
#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio platform report data shared with callers of this public contract.
 */
typedef struct UmiStudioPlatformReport {
    UmiToolchainDiscoveryReport discovery;
    UmiEnvironmentPlan environment;
    size_t framework_capability_count;
    int environment_ready;
} UmiStudioPlatformReport;

/**
 * Provide the studio platform check operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_platform_check(
    UmiStudioServices *services,
    int require_gtk,
    int require_github_cli,
    UmiStudioPlatformReport *out_report
);
/**
 * Release or reset state held by studio platform report so the same storage can be reused
 * safely.
 */
void umi_studio_platform_report_dispose(UmiStudioPlatformReport *report);
/**
 * Provide the studio platform write environment operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_platform_write_environment(
    const UmiStudioPlatformReport *report,
    const char *path
);

#ifdef __cplusplus
}
#endif

#endif
