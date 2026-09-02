/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/ui_catalogue_main.c
 *
 * PURPOSE:
 *   Print the Studio UI catalogue for architecture inspection and automated
 *   validation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include <stdio.h>

#include "umicom/studio/bootstrap.h"
#include "umicom/studio/ui_catalogue.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioUiCatalogue catalogue;
    char report[2048];
    UmiStatus status = umi_studio_bootstrap_create(&bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_bootstrap_start(bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_ui_catalogue_snapshot(umi_studio_bootstrap_ui(bootstrap), &catalogue);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_ui_catalogue_format(&catalogue, report, sizeof(report));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) (void)printf("%s", report);
    /* Use this fallback path when the earlier condition does not apply. */
    else (void)fprintf(stderr, "UI catalogue failed: %s\n", umi_status_text(status));
    umi_studio_bootstrap_destroy(bootstrap);
    return status == UMI_STATUS_OK ? 0 : 1;
}
