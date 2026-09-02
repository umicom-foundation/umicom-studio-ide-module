/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/workspace_layouts.h
 *
 * PURPOSE:
 *   Expose Studio layout seeding plus canonical Framework suite-layout
 *   selection and live-workbench profile registration.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_WORKSPACE_LAYOUTS_H
#define UMICOM_STUDIO_WORKSPACE_LAYOUTS_H
#include "umicom/studio/professional_workspace.h"
#include "umicom/application/suite_layout/suite_layout.h"

/**
 * Provide the studio workspace layouts seed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_layouts_seed(
    UmiStudioProfessionalWorkspace *workspace);
/**
 * Provide the studio workspace layout default operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_layout_default(
    UmiUiWorkspaceLayout *out_layout);
/**
 * Provide the studio workspace layout select operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_layout_select(
    const char *layout_id,
    UmiUiWorkspaceLayout *out_layout);
/**
 * Provide the studio workspace layout register workbench operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_workspace_layout_register_workbench(
    UmiUiWorkbench *workbench,
    int activate_default);

#endif
