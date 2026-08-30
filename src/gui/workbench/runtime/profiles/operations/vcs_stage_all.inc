/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_stage_all.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Stage All" for
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
    "vcs_stage_all",
    "studio.vcs.stage-all",
    "Stage All",
    "Source Control",
    "Stage every working-tree change.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
