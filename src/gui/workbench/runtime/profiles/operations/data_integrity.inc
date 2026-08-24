/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/data_integrity.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Data Server Integrity" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "data_integrity",
    "studio.data.integrity",
    "Data Server Integrity",
    "Data",
    "Inspect Data Server integrity.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE)
