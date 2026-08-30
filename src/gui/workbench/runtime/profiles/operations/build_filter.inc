/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_filter.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Filter Build Workspace" for
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
    "build_filter",
    "studio.build.filter",
    "Filter Build Workspace",
    "Build",
    "Filter build workspace nodes.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
