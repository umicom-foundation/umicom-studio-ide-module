/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/security_commands.c
 *
 * PURPOSE:
 *   Implement the security commands behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Security Centre commands v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/security_commands.h"
#include "umicom/studio/security_approvals.h"
#include "umicom/studio/security_identities.h"
#include "umicom/studio/security_packages.h"
#include "umicom/studio/security_secrets.h"
#include <string.h>

/* Provide the seed identity operation used by this module and its client applications. */
static UmiStatus seed_identity(UmiStudioSecurityCentre *centre)
{
    const UmiSecurityIdentityProfile identity = {
        .identity_id = "sammy",
        .display_name = "Sammy Hegab",
        .organisation = "Umicom Foundation",
        .kind = UMI_IDENTITY_HUMAN,
        .assurance_level = 3U,
        .enabled = true,
        .revision = 1U
    };
    return umi_studio_security_identity_add(centre,&identity);
}

/* Provide the seed permissions operation used by this module and its client applications. */
static UmiStatus seed_permissions(UmiStudioSecurityCentre *centre)
{
    const UmiSecurityPermissionDescriptor build = {
        .name = "project.build",
        .description = "Build a project in a trusted Studio workspace",
        .risk = UMI_SECURITY_RISK_HIGH,
        .trusted_workspace_required = true,
        .approval_required = false
    };
    const UmiSecurityPermissionDescriptor plugin = {
        .name = "plugin.install",
        .description = "Install a reviewed plug-in into Studio",
        .risk = UMI_SECURITY_RISK_CRITICAL,
        .trusted_workspace_required = true,
        .approval_required = true
    };
    UmiStatus status = umi_studio_security_permission_add(centre,&build);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_security_permission_add(centre,&plugin);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_security_role_define(centre,"developer","Trusted Studio developer");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_security_role_grant(centre,"developer","project.build");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_security_role_grant(centre,"developer","plugin.install");
    return status;
}

/* Provide the seed assignment operation used by this module and its client applications. */
static UmiStatus seed_assignment(UmiStudioSecurityCentre *centre)
{
    const UmiSecurityRoleAssignment assignment = {
        .assignment_id = "sammy-developer",
        .identity_id = "sammy",
        .role_id = "developer",
        .resource_scope = "workspace://",
        .expires_at_ns = 0U,
        .enabled = true
    };
    return umi_studio_security_role_assign(centre,&assignment);
}

/* Provide the seed secret operation used by this module and its client applications. */
static UmiStatus seed_secret(UmiStudioSecurityCentre *centre)
{
    const UmiSecuritySecretMetadata metadata = {
        .secret = { .reference = "vault://studio/signing-key" },
        .purpose = "Studio package signing key reference",
        .rotation_due_ns = 1000U,
        .enabled = true
    };
    return umi_studio_security_secret_add(centre,&metadata);
}

/* Provide the seed signer operation used by this module and its client applications. */
static UmiStatus seed_signer(UmiStudioSecurityCentre *centre)
{
    const UmiSecurityTrustedSigner signer = {
        .signer_id = "umicom-release",
        .fingerprint = "UMICOM-RELEASE-SIGNER-01",
        .enabled = true
    };
    return umi_studio_security_signer_add(centre,&signer);
}

/* Provide the seed approval operation used by this module and its client applications. */
static UmiStatus seed_approval(UmiStudioSecurityCentre *centre)
{
    const UmiSecurityApproval approval = {
        .approval_id = "approve-plugin-install",
        .subject_kind = UMI_SECURITY_APPROVAL_PLUGIN,
        .subject_id = "plugin.install",
        .requested_by = "sammy",
        .state = UMI_SECURITY_APPROVAL_PENDING,
        .requested_at_ns = 1U,
        .expires_at_ns = 1000U,
        .reason = "Review plug-in installation"
    };
    UmiStatus status = umi_studio_security_approval_request(centre,&approval);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_security_approval_decide(centre,
                                                     "approve-plugin-install",
                                                     "security-admin",
                                                     UMI_SECURITY_APPROVAL_APPROVED,
                                                     "Reviewed and approved");
    }
    return status;
}

/*
 * Provide the studio security seed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_seed(UmiStudioSecurityCentre *centre)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = seed_identity(centre);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = seed_permissions(centre);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = seed_assignment(centre);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = seed_secret(centre);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = seed_signer(centre);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = seed_approval(centre);
    return status;
}

/* Provide the authorise operation used by this module and its client applications. */
static UmiStatus authorise(UmiStudioSecurityCentre *centre,
                           const char *permission,
                           const char *approval_id,
                           UmiSecurityGovernanceDecision *out_decision)
{
    UmiSecurityAccessRequest request = {
        .identity_id = "sammy",
        .role_id = "developer",
        .resource = "workspace://demo",
        .workspace_trust = UMI_WORKSPACE_TRUSTED,
        .environment = UMI_SECURITY_ENV_DEVELOPMENT,
        .now_ns = 10U,
        .correlation_id = 56U
    };
    (void)memcpy(request.permission,permission,strlen(permission) + 1U);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (approval_id != NULL) {
        (void)memcpy(request.approval_id,approval_id,strlen(approval_id) + 1U);
    }
    return umi_studio_security_centre_authorise(centre,&request,out_decision);
}

/*
 * Perform studio security through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_security_execute(UmiStudioSecurityCentre *centre,
                                      UmiStudioSecurityCommand command,
                                      UmiSecurityGovernanceDecision *out_decision)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Select the behaviour associated with the requested command or state value. */
    switch (command) {
        case UMI_STUDIO_SECURITY_COMMAND_SEED:
            return umi_studio_security_seed(centre);
        case UMI_STUDIO_SECURITY_COMMAND_AUTHORISE_BUILD:
            /*
             * Protect caller-owned memory by checking that required state is available before it is
             * used.
             */
            if (out_decision == NULL) return UMI_STATUS_INVALID_ARGUMENT;
            return authorise(centre,"project.build",NULL,out_decision);
        case UMI_STUDIO_SECURITY_COMMAND_AUTHORISE_PLUGIN:
            /*
             * Protect caller-owned memory by checking that required state is available before it is
             * used.
             */
            if (out_decision == NULL) return UMI_STATUS_INVALID_ARGUMENT;
            return authorise(centre,"plugin.install","approve-plugin-install",out_decision);
        default:
            return UMI_STATUS_INVALID_ARGUMENT;
    }
}
