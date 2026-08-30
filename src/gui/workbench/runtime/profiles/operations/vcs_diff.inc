/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_diff.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Open Diff" for
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
    "vcs_diff",
    "studio.vcs.diff",
    "Open Diff",
    "Source Control",
    "Open the diff for a repository-relative path.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_PATH)
