/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_set_commit_message.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Set Commit Message" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "vcs_set_commit_message",
    "studio.vcs.set-commit-message",
    "Set Commit Message",
    "Source Control",
    "Set the commit-composition message.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
