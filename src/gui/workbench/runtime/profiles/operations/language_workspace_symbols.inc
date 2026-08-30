/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/language_workspace_symbols.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Workspace Symbols" for
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
    "language_workspace_symbols",
    "studio.language.workspace-symbols",
    "Workspace Symbols",
    "Language",
    "Search workspace symbols.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
