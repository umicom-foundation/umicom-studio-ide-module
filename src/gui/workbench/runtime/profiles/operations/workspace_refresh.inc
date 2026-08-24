/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/workspace_refresh.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Refresh Workspace" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "workspace_refresh",
    "studio.workspace.refresh",
    "Refresh Workspace",
    "Workspace",
    "Refresh the active workspace graph and indexes.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
