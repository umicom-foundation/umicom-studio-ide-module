/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/terminal_close.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Close Terminal" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "terminal_close",
    "studio.terminal.close",
    "Close Terminal",
    "Terminal",
    "Close the active terminal session.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
