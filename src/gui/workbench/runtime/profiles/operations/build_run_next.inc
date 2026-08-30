/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_run_next.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Run Next Build Task" for
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
    "build_run_next",
    "studio.build.run-next",
    "Run Next Build Task",
    "Build",
    "Run the next eligible build task.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
