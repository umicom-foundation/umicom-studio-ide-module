/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/workspace_layouts.h
 *
 * PURPOSE:
 *   Expose Studio layout seeding plus canonical Framework suite-layout selection.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_WORKSPACE_LAYOUTS_H
#define UMICOM_STUDIO_WORKSPACE_LAYOUTS_H
#include "umicom/studio/professional_workspace.h"
#include "umicom/application/suite_layout/suite_layout.h"

UmiStatus umi_studio_workspace_layouts_seed(
    UmiStudioProfessionalWorkspace *workspace);
UmiStatus umi_studio_workspace_layout_default(
    UmiUiWorkspaceLayout *out_layout);
UmiStatus umi_studio_workspace_layout_select(
    const char *layout_id,
    UmiUiWorkspaceLayout *out_layout);

#endif
