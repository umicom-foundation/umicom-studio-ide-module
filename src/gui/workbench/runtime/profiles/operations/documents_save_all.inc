/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/documents_save_all.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Save All Documents" for
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
    "documents_save_all",
    "studio.documents.save-all",
    "Save All Documents",
    "File",
    "Save every dirty document.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
