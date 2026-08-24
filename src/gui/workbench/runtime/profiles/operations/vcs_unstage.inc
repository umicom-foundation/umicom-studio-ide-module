/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_unstage.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Unstage Path" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "vcs_unstage",
    "studio.vcs.unstage",
    "Unstage Path",
    "Source Control",
    "Unstage a repository-relative path.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_PATH)
