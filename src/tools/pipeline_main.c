/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/pipeline_main.c
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
#include "umicom/studio/delivery_platform.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void) {
    UmiStudioDeliveryPlatform *platform = NULL;
    UmiDeliveryPipeline *pipeline;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_studio_delivery_platform_create(&platform) != UMI_STATUS_OK) return 1;
    pipeline = umi_studio_delivery_pipeline(platform);
    printf("Pipeline %s has %zu gate(s)\n", pipeline->pipeline_id, pipeline->gate_count);
    umi_studio_delivery_platform_destroy(platform);
    return 0;
}
