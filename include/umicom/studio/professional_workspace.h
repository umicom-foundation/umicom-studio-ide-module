/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/professional_workspace.h
 *
 * PURPOSE:
 *   Publish Studio's public professional workspace contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_PROFESSIONAL_WORKSPACE_H
#define UMICOM_STUDIO_PROFESSIONAL_WORKSPACE_H
#include "umicom/studio/services.h"
#include "umicom/ui/professional_workspace.h"
/**
 * Represent the studio professional workspace data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioProfessionalWorkspace UmiStudioProfessionalWorkspace;
/**
 * Represent the studio professional workspace snapshot data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioProfessionalWorkspaceSnapshot {
    UmiUiWorkspaceCustomisationSnapshot customisation;
    char active_view[UMI_UI_WORKSPACE_LAYOUT_ID_CAPACITY];
    uint64_t revision;
} UmiStudioProfessionalWorkspaceSnapshot;
/**
 * Initialise studio professional workspace from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_professional_workspace_create(UmiStudioServices *services,UmiStudioProfessionalWorkspace **out_workspace);
/**
 * Release or reset state held by studio professional workspace so the same storage can be
 * reused safely.
 */
void umi_studio_professional_workspace_destroy(UmiStudioProfessionalWorkspace *workspace);
/**
 * Provide the studio professional workspace activate view operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_professional_workspace_activate_view(UmiStudioProfessionalWorkspace *workspace,const char *view_id);
/**
 * Provide the studio professional workspace snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_professional_workspace_snapshot(const UmiStudioProfessionalWorkspace *workspace,UmiStudioProfessionalWorkspaceSnapshot *out_snapshot);
/**
 * Provide the studio professional workspace model operation used by this module and its
 * client applications.
 */
UmiUiWorkspaceCustomisation *umi_studio_professional_workspace_model(UmiStudioProfessionalWorkspace *workspace);
/* Prepare an empty workspace for the native IDE using Framework's real Studio
 * surface and preset catalogues. The frontend owns this workspace and borrows
 * the model; its services must outlive it. Existing layouts are never replaced.
 * This is separate from the general desktop workspace used by other tools. */
UmiStatus umi_studio_professional_workspace_seed_native(
    UmiStudioProfessionalWorkspace *workspace);
#endif
