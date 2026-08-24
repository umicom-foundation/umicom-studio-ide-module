/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_add_breakpoint.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Add Breakpoint" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "debug_add_breakpoint",
    "studio.debug.add-breakpoint",
    "Add Breakpoint",
    "Debug",
    "Add a breakpoint using the command argument contract.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
