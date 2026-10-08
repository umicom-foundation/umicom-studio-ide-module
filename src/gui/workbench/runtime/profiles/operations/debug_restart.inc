/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_restart.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Restart Debugging" for
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
UMI_STUDIO_GTK_OPERATION_PROFILE("debug_restart", "studio.debug.restart", "Restart Debugging", "Debug",
                                 "Restart through the active adapter using its current launch configuration.",
                                 UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
