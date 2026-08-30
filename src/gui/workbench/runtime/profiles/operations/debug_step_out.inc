/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_step_out.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Step Out" for
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
    "debug_step_out",
    "studio.debug.step-out",
    "Step Out",
    "Debug",
    "Step out in a debugger thread.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_THREAD)
