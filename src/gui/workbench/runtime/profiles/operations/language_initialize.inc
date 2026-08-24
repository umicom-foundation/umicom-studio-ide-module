/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/language_initialize.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Initialize Language Service" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "language_initialize",
    "studio.language.initialize",
    "Initialize Language Service",
    "Language",
    "Initialize the active language server session.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
