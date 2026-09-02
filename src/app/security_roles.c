/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/security_roles.c
 *
 * PURPOSE:
 *   Implement the security roles behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Security role and permission administration v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/security_roles.h"
/* Provide the revised operation used by this module and its client applications. */
static UmiStatus revised(UmiStudioSecurityCentre *centre,UmiStatus status)
{
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) { centre->governance.revision += 1U; centre->revision += 1U; }
    return status;
}
/*
 * Add studio security permission only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_security_permission_add(UmiStudioSecurityCentre *centre,const UmiSecurityPermissionDescriptor *permission) { return centre == NULL ? UMI_STATUS_INVALID_ARGUMENT : revised(centre,umi_security_permission_catalogue_register(&centre->governance.permissions,permission)); }
/*
 * Provide the studio security role define operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_role_define(UmiStudioSecurityCentre *centre,const char *role_id,const char *description) { return centre == NULL ? UMI_STATUS_INVALID_ARGUMENT : revised(centre,umi_security_role_policy_define(&centre->governance.roles,role_id,description)); }
/*
 * Provide the studio security role grant operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_role_grant(UmiStudioSecurityCentre *centre,const char *role_id,const char *permission) { return centre == NULL ? UMI_STATUS_INVALID_ARGUMENT : revised(centre,umi_security_role_policy_grant(&centre->governance.roles,role_id,permission)); }
/*
 * Copy studio security role into module-owned storage so callers keep ownership of their
 * input values.
 */
UmiStatus umi_studio_security_role_assign(UmiStudioSecurityCentre *centre,const UmiSecurityRoleAssignment *assignment) { return centre == NULL ? UMI_STATUS_INVALID_ARGUMENT : revised(centre,umi_security_role_assignment_add(&centre->governance.assignments,assignment)); }
/*
 * Provide the studio security centre authorise operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_security_centre_authorise(UmiStudioSecurityCentre *centre,const UmiSecurityAccessRequest *request,UmiSecurityGovernanceDecision *out_decision) { return centre == NULL ? UMI_STATUS_INVALID_ARGUMENT : revised(centre,umi_security_governance_authorise(&centre->governance,request,out_decision)); }
