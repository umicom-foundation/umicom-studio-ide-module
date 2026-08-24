/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_diff_selected.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Diff Selected Change" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "vcs_diff_selected",
    "studio.vcs.diff-selected",
    "Diff Selected Change",
    "Source Control",
    "Open the selected working-tree diff.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
