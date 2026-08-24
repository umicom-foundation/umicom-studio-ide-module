/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/build_configure.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Configure" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "build_configure",
    "studio.build.configure",
    "Configure",
    "Build",
    "Configure the active build profile.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
