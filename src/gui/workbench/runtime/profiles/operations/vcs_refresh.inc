/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_refresh.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Refresh Source Control" for
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
    "vcs_refresh",
    "studio.vcs.refresh",
    "Refresh Source Control",
    "Source Control",
    "Refresh repository status and history.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
