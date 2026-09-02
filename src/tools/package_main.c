/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/package_main.c
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
#include "umicom/studio/delivery_package.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void) {
    UmiPackageSpec spec;
    UmiPackageResult result;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_studio_package_spec("stage", "dist/umicom-studio", &spec) != UMI_STATUS_OK) return 1;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_studio_package_directory(&spec, &result) != UMI_STATUS_OK) return 2;
    printf("Package: %s\n", result.output_path);
    printf("Format: %s\n", umi_package_format_text(spec.format));
    return result.succeeded ? 0 : 3;
}
