/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/tools/integration_designer_main.c
 *
 * PURPOSE:
 *   Provide the integration designer main development utility for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Native Integration Designer command | Sammy Hegab | Umicom Foundation | MIT */
#include <stdio.h>
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/integration_designer_commands.h"
#include "umicom/studio/integration_designer_monitoring.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioIntegrationDesignerCentre *centre = NULL;
    UmiIntegrationSimulation simulation;
    UmiStudioIntegrationDesignerHealth health;
    UmiStatus status = umi_studio_bootstrap_create(&bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_integration_designer_create(umi_studio_bootstrap_services(bootstrap),&centre);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_integration_designer_execute(centre,UMI_STUDIO_INTEGRATION_COMMAND_SEED_SAMPLE,NULL);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_integration_designer_execute(centre,UMI_STUDIO_INTEGRATION_COMMAND_SIMULATE,&simulation);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_integration_designer_health(centre,&health);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) (void)printf("Integration Designer: %zu connection(s), %zu successful run(s), %zu step(s)\n",health.enabled_connections,health.runs.succeeded,simulation.trace_count);
    umi_studio_integration_designer_destroy(centre);
    umi_studio_bootstrap_destroy(bootstrap);
    return status == UMI_STATUS_OK ? 0 : 1;
}
