/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_select_branch.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Select Repository Branch" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "vcs_select_branch",
    "studio.vcs.select-branch",
    "Select Repository Branch",
    "Source Control",
    "Select a branch for inspection.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
