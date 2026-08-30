/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_select_operation.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Select Build Operation" for
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
    "build_select_operation",
    "studio.build.select-operation",
    "Select Build Operation",
    "Build",
    "Select a build operation by stable id.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
