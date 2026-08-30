/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_clear_console.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Clear Debug Console" for
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
    "debug_clear_console",
    "studio.debug.clear-console",
    "Clear Debug Console",
    "Debug",
    "Clear retained debug console output.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
