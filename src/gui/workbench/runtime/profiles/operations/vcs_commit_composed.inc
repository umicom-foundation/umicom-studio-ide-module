/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_commit_composed.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Commit Composed Message" for
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
    "vcs_commit_composed",
    "studio.vcs.commit-composed",
    "Commit Composed Message",
    "Source Control",
    "Commit using the current workspace composition.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
