/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/process_cancel.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Cancel Process" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "process_cancel",
    "studio.process.cancel",
    "Cancel Process",
    "Processes",
    "Cancel a supervised process by id.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
