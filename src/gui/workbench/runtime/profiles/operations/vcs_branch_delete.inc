/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_branch_delete.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Delete Branch" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "vcs_branch_delete",
    "studio.vcs.branch-delete",
    "Delete Branch",
    "Source Control",
    "Delete a branch.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
