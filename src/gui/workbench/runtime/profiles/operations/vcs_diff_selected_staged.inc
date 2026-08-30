/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_diff_selected_staged.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Diff Selected Staged Change" for
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
    "vcs_diff_selected_staged",
    "studio.vcs.diff-selected-staged",
    "Diff Selected Staged Change",
    "Source Control",
    "Open the selected staged diff.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
