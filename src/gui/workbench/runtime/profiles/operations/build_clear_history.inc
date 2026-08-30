/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_clear_history.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Clear Build History" for
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
    "build_clear_history",
    "studio.build.clear-history",
    "Clear Build History",
    "Build",
    "Clear retained build operation history.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
