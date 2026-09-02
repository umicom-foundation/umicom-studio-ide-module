/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/tools/security_centre_main.c
 *
 * PURPOSE:
 *   Provide the security centre main development utility for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Native Security Centre command | Sammy Hegab | Umicom Foundation | MIT */
#include <stdio.h>
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/security_commands.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioSecurityCentre *centre = NULL;
    UmiStudioSecurityCentreSnapshot snapshot;
    UmiSecurityGovernanceDecision decision;
    UmiStatus status = umi_studio_bootstrap_create(&bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_security_centre_create(
            umi_studio_bootstrap_services(bootstrap),&centre);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_security_execute(
            centre,UMI_STUDIO_SECURITY_COMMAND_SEED,NULL);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_security_execute(
            centre,UMI_STUDIO_SECURITY_COMMAND_AUTHORISE_BUILD,&decision);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK && !decision.allowed) {
        status = UMI_STATUS_PERMISSION_DENIED;
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_security_centre_snapshot(centre,10U,&snapshot);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)printf("Security Centre: %zu identities, %zu approvals, %zu audit events\n",
                     snapshot.governance.identities,
                     snapshot.governance.approvals,
                     snapshot.governance.audit_entries);
    }
    umi_studio_security_centre_destroy(centre);
    umi_studio_bootstrap_destroy(bootstrap);
    return status == UMI_STATUS_OK ? 0 : 1;
}
