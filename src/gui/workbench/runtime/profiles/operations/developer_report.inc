/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/developer_report.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Developer Platform Report" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "developer_report",
    "studio.developer.report",
    "Developer Platform Report",
    "Developer",
    "Inspect the composed developer platform.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
