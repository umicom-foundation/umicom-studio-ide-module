/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_run.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Run" for
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
    "build_run",
    "studio.build.run",
    "Run",
    "Build",
    "Run the active build profile.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
