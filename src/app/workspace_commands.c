/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/workspace_commands.c
 *
 * PURPOSE:
 *   Implement the workspace commands behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/workspace_commands.h"
#include "umicom/studio/workspace_catalogue.h"
#include "umicom/studio/workspace_groups.h"
#include "umicom/studio/workspace_layouts.h"
/*
 * Provide the studio workspace seed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_seed(UmiStudioProfessionalWorkspace *workspace)
{
    UmiStatus status;
    status = umi_studio_workspace_catalogue_seed(workspace); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_studio_workspace_layouts_seed(workspace); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_studio_workspace_groups_seed(workspace); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    return umi_studio_workspace_theme_apply(workspace,UMI_STUDIO_WORKSPACE_THEME_DARK,UMI_UI_DENSITY_COMPACT,1.0);
}
/*
 * Perform studio workspace through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_workspace_execute(UmiStudioProfessionalWorkspace *workspace,UmiStudioWorkspaceCommand command)
{
    UmiUiWorkspaceCustomisation *model = umi_studio_professional_workspace_model(workspace);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (model == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Select the behaviour associated with the requested command or state value. */
    switch (command) {
        case UMI_STUDIO_WORKSPACE_COMMAND_SEED: return umi_studio_workspace_seed(workspace);
        case UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_DEVELOP: return umi_ui_workspace_customisation_activate(model,"develop");
        case UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_FOCUS: return umi_ui_workspace_customisation_activate(model,"focus");
        case UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_DEBUG: return umi_ui_workspace_customisation_activate(model,"debug");
        case UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_OPERATIONS: return umi_ui_workspace_customisation_activate(model,"operations");
        case UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_TRADING: return umi_ui_workspace_customisation_activate(model,"trading");
        case UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_COMPARE: return umi_ui_workspace_customisation_activate(model,"compare");
        case UMI_STUDIO_WORKSPACE_COMMAND_UNLOCK:
            return umi_ui_workspace_customisation_begin_edit(model);
        case UMI_STUDIO_WORKSPACE_COMMAND_LOCK:
            return umi_ui_workspace_customisation_commit_edit(model);
        case UMI_STUDIO_WORKSPACE_COMMAND_THEME_LIGHT: return umi_studio_workspace_theme_apply(workspace,UMI_STUDIO_WORKSPACE_THEME_LIGHT,UMI_UI_DENSITY_COMFORTABLE,1.0);
        case UMI_STUDIO_WORKSPACE_COMMAND_THEME_DARK: return umi_studio_workspace_theme_apply(workspace,UMI_STUDIO_WORKSPACE_THEME_DARK,UMI_UI_DENSITY_COMFORTABLE,1.0);
        case UMI_STUDIO_WORKSPACE_COMMAND_THEME_HIGH_CONTRAST: return umi_studio_workspace_theme_apply(workspace,UMI_STUDIO_WORKSPACE_THEME_HIGH_CONTRAST,UMI_UI_DENSITY_COMFORTABLE,1.1);
        default: return UMI_STATUS_NOT_IMPLEMENTED;
    }
}

/* Forward Studio panel settings to the reusable Framework transaction. */
UmiStatus umi_studio_workspace_apply_panel_settings(
    UmiStudioProfessionalWorkspace *workspace,
    const UmiUiWorkspacePanelSettings *settings)
{
    UmiUiWorkspaceCustomisation *model =
        umi_studio_professional_workspace_model(workspace);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (model == NULL || settings == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    /* Studio contributes the selected panel identity and user choices only;
     * Framework owns the transactional docking, grouping and rollback rules. */
    return umi_ui_workspace_customisation_apply_panel_settings(
        model, settings);
}
