/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/security_roles.h
 *
 * PURPOSE:
 *   Publish Studio's public security roles contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SECURITY_ROLES_H
#define UMICOM_STUDIO_SECURITY_ROLES_H
#include "umicom/studio/security_centre.h"
/**
 * Add studio security permission only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_security_permission_add(UmiStudioSecurityCentre *centre,const UmiSecurityPermissionDescriptor *permission);
/**
 * Provide the studio security role define operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_role_define(UmiStudioSecurityCentre *centre,const char *role_id,const char *description);
/**
 * Provide the studio security role grant operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_role_grant(UmiStudioSecurityCentre *centre,const char *role_id,const char *permission);
/**
 * Copy studio security role into module-owned storage so callers keep ownership of their
 * input values.
 */
UmiStatus umi_studio_security_role_assign(UmiStudioSecurityCentre *centre,const UmiSecurityRoleAssignment *assignment);
/**
 * Provide the studio security centre authorise operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_security_centre_authorise(UmiStudioSecurityCentre *centre,const UmiSecurityAccessRequest *request,UmiSecurityGovernanceDecision *out_decision);
#endif
