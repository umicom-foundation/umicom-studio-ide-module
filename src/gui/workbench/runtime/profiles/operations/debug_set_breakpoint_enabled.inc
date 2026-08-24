/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_set_breakpoint_enabled.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Enable/Disable Breakpoint" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "debug_set_breakpoint_enabled",
    "studio.debug.set-breakpoint-enabled",
    "Enable/Disable Breakpoint",
    "Debug",
    "Change breakpoint enabled state.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
