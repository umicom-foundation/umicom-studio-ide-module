/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_select_remote.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Select Repository Remote" for
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
    "vcs_select_remote",
    "studio.vcs.select-remote",
    "Select Repository Remote",
    "Source Control",
    "Select a remote for inspection.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
