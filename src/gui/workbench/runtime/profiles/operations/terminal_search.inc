/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/terminal_search.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Search Terminal" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "terminal_search",
    "studio.terminal.search",
    "Search Terminal",
    "Terminal",
    "Search the terminal transcript.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
