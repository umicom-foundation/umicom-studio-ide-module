/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_select_scope.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Select Scope" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "debug_select_scope",
    "studio.debug.select-scope",
    "Select Scope",
    "Debug",
    "Select a debugger variable scope.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
