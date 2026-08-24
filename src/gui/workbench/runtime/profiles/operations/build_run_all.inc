/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_run_all.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Run Build Graph" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "build_run_all",
    "studio.build.run-all",
    "Run Build Graph",
    "Build",
    "Run every eligible task in the build graph.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
