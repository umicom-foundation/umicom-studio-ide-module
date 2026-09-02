/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/security_views.h
 *
 * PURPOSE:
 *   Publish Studio's public security views contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SECURITY_VIEWS_H
#define UMICOM_STUDIO_SECURITY_VIEWS_H
#include "umicom/studio/security_centre.h"
/**
 * List the named studio security view kind values accepted by this public contract.
 */
typedef enum UmiStudioSecurityViewKind {
    UMI_STUDIO_SECURITY_VIEW_OVERVIEW = 1,
    UMI_STUDIO_SECURITY_VIEW_IDENTITIES,
    UMI_STUDIO_SECURITY_VIEW_ROLES,
    UMI_STUDIO_SECURITY_VIEW_WORKSPACE_TRUST,
    UMI_STUDIO_SECURITY_VIEW_SECRETS,
    UMI_STUDIO_SECURITY_VIEW_APPROVALS,
    UMI_STUDIO_SECURITY_VIEW_PACKAGE_TRUST,
    UMI_STUDIO_SECURITY_VIEW_AUDIT
} UmiStudioSecurityViewKind;
/**
 * Represent the studio security view data shared with callers of this public contract.
 */
typedef struct UmiStudioSecurityView { UmiStudioSecurityViewKind kind; char id[UMI_SECURITY_GOVERNANCE_ID_CAPACITY]; char title[UMI_SECURITY_GOVERNANCE_NAME_CAPACITY]; char summary[UMI_SECURITY_GOVERNANCE_TEXT_CAPACITY]; size_t item_count; uint64_t revision; } UmiStudioSecurityView;
/**
 * Provide the studio security view id operation used by this module and its client
 * applications.
 */
const char *umi_studio_security_view_id(UmiStudioSecurityViewKind kind);
/**
 * Provide the studio security view build operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_view_build(const UmiStudioSecurityCentre *centre,uint64_t now_ns,UmiStudioSecurityViewKind kind,UmiStudioSecurityView *out_view);
#endif
