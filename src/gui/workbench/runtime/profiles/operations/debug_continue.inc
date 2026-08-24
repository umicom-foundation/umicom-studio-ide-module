/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_continue.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Continue" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "debug_continue",
    "studio.debug.continue",
    "Continue",
    "Debug",
    "Continue a debugger thread.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_THREAD)
