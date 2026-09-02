/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/helix_agent_runtime_main.c
 * PURPOSE: Exercise the governed Helix Agent Runtime v2 composition natively.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* This smoke command uses the same approval and adapter paths
 * that graphical workbench actions use. */
#include <stdio.h>
#include "umicom/studio/ai_platform.h"
#include "umicom/studio/helix_agent_commands.h"
#include "umicom/studio/helix_agent_operations.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioAiPlatform *platform = NULL;
    UmiStudioHelixAgentCentre *centre;
    UmiHelixAction action;
    char evidence[512];
    char status_text[512];
    UmiStatus status = umi_studio_ai_platform_create(&platform);
    centre = status == UMI_STATUS_OK
        ? umi_studio_ai_platform_helix_agent_centre(platform) : NULL;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status =
        umi_studio_helix_agent_review_action("workspace://candidate", &action);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_helix_agent_plan(
        centre, "studio.helix.smoke", "Review isolated candidate",
        "smoke-plan", &action);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_helix_agent_approve(
        centre, "studio.user", "Native smoke action approved");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_helix_agent_run(
        centre, evidence, sizeof(evidence));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_helix_agent_status(
        centre, status_text, sizeof(status_text));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) printf("%s\nEvidence: %s\n", status_text, evidence);
    umi_studio_ai_platform_destroy(platform);
    return status == UMI_STATUS_OK ? 0 : 1;
}
