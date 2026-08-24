/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_stage_selected.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Stage Selected Change" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "vcs_stage_selected",
    "studio.vcs.stage-selected",
    "Stage Selected Change",
    "Source Control",
    "Stage the selected change.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
