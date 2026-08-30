/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/terminal_split_horizontal.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Split Terminal Horizontally" for
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
    "terminal_split_horizontal",
    "studio.terminal.split-horizontal",
    "Split Terminal Horizontally",
    "Terminal",
    "Split the terminal horizontally.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
