/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_push.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Push" for
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
    "vcs_push",
    "studio.vcs.push",
    "Push",
    "Source Control",
    "Push the current branch.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
