/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/security_report.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Security Report" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "security_report",
    "studio.security.report",
    "Security Report",
    "Security",
    "Inspect security/identity state.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
