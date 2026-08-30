/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_select_frame.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Select Stack Frame" for
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
    "debug_select_frame",
    "studio.debug.select-frame",
    "Select Stack Frame",
    "Debug",
    "Select a debugger stack frame.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
