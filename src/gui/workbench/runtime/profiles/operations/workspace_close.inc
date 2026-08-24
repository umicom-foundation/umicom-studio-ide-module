/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/workspace_close.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Close Workspace" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "workspace_close",
    "studio.workspace.close",
    "Close Workspace",
    "Workspace",
    "Close the active workspace.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
