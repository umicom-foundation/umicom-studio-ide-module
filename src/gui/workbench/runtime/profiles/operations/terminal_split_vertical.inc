/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/terminal_split_vertical.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Split Terminal Vertically" for
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
    "terminal_split_vertical",
    "studio.terminal.split-vertical",
    "Split Terminal Vertically",
    "Terminal",
    "Split the terminal vertically.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
