/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/vcs_discard_selected.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Discard Selected Change" for
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
    "vcs_discard_selected",
    "studio.vcs.discard-selected",
    "Discard Selected Change",
    "Source Control",
    "Discard the selected change.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
