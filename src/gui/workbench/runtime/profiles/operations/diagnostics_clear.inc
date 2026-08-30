/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/diagnostics_clear.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Clear Diagnostics" for
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
    "diagnostics_clear",
    "studio.diagnostics.clear",
    "Clear Diagnostics",
    "Problems",
    "Clear retained diagnostics.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
