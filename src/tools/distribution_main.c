/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/distribution_main.c
 *
 * PURPOSE:
 *   Provide a native Studio command for the Framework-backed application-delivery platform.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This command is a thin product-facing wrapper; the reusable delivery logic lives in Umicom Framework.
 */

#include <stdio.h>
#include "umicom/studio/delivery_distribution.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void) {
    UmiPlatformMatrix matrix;
    UmiRuntimeBundle bundle;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_studio_distribution_matrix(&matrix) != UMI_STATUS_OK) return 1;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_studio_runtime_bundle(&bundle) != UMI_STATUS_OK) return 2;
    printf("Distribution targets: %zu\n", matrix.count);
    printf("Runtime bundle files: %zu\n", bundle.count);
    return 0;
}
