/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_retry.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Retry Build Operation" for
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
    "build_retry",
    "studio.build.retry",
    "Retry Build Operation",
    "Build",
    "Retry the selected failed build operation.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
