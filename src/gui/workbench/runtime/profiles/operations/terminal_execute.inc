/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/terminal_execute.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Execute Terminal Command" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "terminal_execute",
    "studio.terminal.execute",
    "Execute Terminal Command",
    "Terminal",
    "Execute a command in the primary terminal.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
