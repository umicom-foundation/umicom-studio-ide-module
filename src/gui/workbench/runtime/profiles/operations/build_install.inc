/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_install.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Install" for
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
    "build_install",
    "studio.build.install",
    "Install",
    "Build",
    "Install the active build profile.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
