/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_refresh.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Refresh Build Workspace" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "build_refresh",
    "studio.build.refresh",
    "Refresh Build Workspace",
    "Build",
    "Refresh build workspace state.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
