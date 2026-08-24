/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_invalidate.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Invalidate Build Node" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "build_invalidate",
    "studio.build.invalidate",
    "Invalidate Build Node",
    "Build",
    "Invalidate a build node by stable id.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
