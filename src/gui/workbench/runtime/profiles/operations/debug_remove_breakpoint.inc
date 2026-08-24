/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_remove_breakpoint.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Remove Breakpoint" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "debug_remove_breakpoint",
    "studio.debug.remove-breakpoint",
    "Remove Breakpoint",
    "Debug",
    "Remove a breakpoint by stable id.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
