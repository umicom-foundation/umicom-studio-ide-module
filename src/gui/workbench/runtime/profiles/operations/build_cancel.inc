/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_cancel.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Cancel Build" for
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
    "build_cancel",
    "studio.build.cancel",
    "Cancel Build",
    "Build",
    "Cancel the current build operation.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
