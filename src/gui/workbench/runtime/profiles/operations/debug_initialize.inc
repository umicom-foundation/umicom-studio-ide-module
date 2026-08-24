/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/debug_initialize.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Initialize Debug Adapter" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "debug_initialize",
    "studio.debug.initialize",
    "Initialize Debug Adapter",
    "Debug",
    "Initialize the configured debug adapter.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
