/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/security_centre.h
 *
 * PURPOSE:
 *   Publish Studio's public security centre contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SECURITY_CENTRE_H
#define UMICOM_STUDIO_SECURITY_CENTRE_H
#include "umicom/studio/services.h"
#include "umicom/security/security_admin.h"
/**
 * Represent the studio security centre data shared with callers of this public contract.
 */
typedef struct UmiStudioSecurityCentre {
    UmiStudioServices *services;
    UmiSecurityGovernanceCentre governance;
    char active_view[UMI_SECURITY_GOVERNANCE_ID_CAPACITY];
    uint64_t revision;
} UmiStudioSecurityCentre;
/**
 * Represent the studio security centre snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioSecurityCentreSnapshot {
    UmiSecurityGovernanceSnapshot governance;
    size_t enabled_identities;
    size_t pending_approvals;
    size_t secrets_due_rotation;
    char active_view[UMI_SECURITY_GOVERNANCE_ID_CAPACITY];
    uint64_t revision;
} UmiStudioSecurityCentreSnapshot;
/**
 * Initialise studio security centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_security_centre_create(UmiStudioServices *services,UmiStudioSecurityCentre **out_centre);
/**
 * Release or reset state held by studio security centre so the same storage can be reused
 * safely.
 */
void umi_studio_security_centre_destroy(UmiStudioSecurityCentre *centre);
/**
 * Provide the studio security centre activate operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_centre_activate(UmiStudioSecurityCentre *centre,const char *view_id);
/**
 * Provide the studio security centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_centre_snapshot(const UmiStudioSecurityCentre *centre,uint64_t now_ns,UmiStudioSecurityCentreSnapshot *out_snapshot);
#endif
