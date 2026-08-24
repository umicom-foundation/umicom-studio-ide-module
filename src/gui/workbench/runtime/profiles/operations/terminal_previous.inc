/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/terminal_previous.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Previous Terminal" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "terminal_previous",
    "studio.terminal.previous",
    "Previous Terminal",
    "Terminal",
    "Activate the previous terminal session.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
