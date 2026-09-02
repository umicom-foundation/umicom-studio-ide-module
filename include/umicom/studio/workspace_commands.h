/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/workspace_commands.h
 *
 * PURPOSE:
 *   Publish Studio's public workspace commands contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_WORKSPACE_COMMANDS_H
#define UMICOM_STUDIO_WORKSPACE_COMMANDS_H
#include "umicom/studio/workspace_themes.h"
/**
 * List the named studio workspace command values accepted by this public contract.
 */
typedef enum UmiStudioWorkspaceCommand {
    UMI_STUDIO_WORKSPACE_COMMAND_SEED = 1,
    UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_DEVELOP,
    UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_FOCUS,
    UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_DEBUG,
    UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_OPERATIONS,
    UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_TRADING,
    UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_COMPARE,
    UMI_STUDIO_WORKSPACE_COMMAND_UNLOCK,
    UMI_STUDIO_WORKSPACE_COMMAND_LOCK,
    UMI_STUDIO_WORKSPACE_COMMAND_THEME_LIGHT,
    UMI_STUDIO_WORKSPACE_COMMAND_THEME_DARK,
    UMI_STUDIO_WORKSPACE_COMMAND_THEME_HIGH_CONTRAST
} UmiStudioWorkspaceCommand;
/**
 * Provide the studio workspace seed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_seed(UmiStudioProfessionalWorkspace *workspace);
/**
 * Perform studio workspace through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_workspace_execute(UmiStudioProfessionalWorkspace *workspace,UmiStudioWorkspaceCommand command);
/* Forward a complete panel edit to Framework without Studio-owned layout logic. */
UmiStatus umi_studio_workspace_apply_panel_settings(
    UmiStudioProfessionalWorkspace *workspace,
    const UmiUiWorkspacePanelSettings *settings);
#endif
