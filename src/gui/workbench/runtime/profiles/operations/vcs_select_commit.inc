/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_select_commit.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Select Repository Commit" for
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
    "vcs_select_commit",
    "studio.vcs.select-commit",
    "Select Repository Commit",
    "Source Control",
    "Select a commit by id.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
