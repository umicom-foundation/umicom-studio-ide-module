/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_test.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Build Test Phase" for
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
    "build_test",
    "studio.build.test",
    "Build Test Phase",
    "Build",
    "Run the build system test phase.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
