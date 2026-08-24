/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_compile.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Build" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "build_compile",
    "studio.build.compile",
    "Build",
    "Build",
    "Compile the active project.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
