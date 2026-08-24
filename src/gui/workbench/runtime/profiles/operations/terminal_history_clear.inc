/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/terminal_history_clear.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Clear Terminal History" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "terminal_history_clear",
    "studio.terminal.history-clear",
    "Clear Terminal History",
    "Terminal",
    "Clear terminal history.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
