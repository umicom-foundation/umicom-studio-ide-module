/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/terminal_clear.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Clear Terminal" for
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
    "terminal_clear",
    "studio.terminal.clear",
    "Clear Terminal",
    "Terminal",
    "Clear the primary terminal transcript.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
