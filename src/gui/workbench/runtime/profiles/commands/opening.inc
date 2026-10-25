/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/profiles/commands/opening.inc
 *
 * PURPOSE:
 *   Expose queued opening cancellation and retained results in native command discovery.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


UMI_STUDIO_GTK_COMMAND_PROFILE(
    "cancel_opening", "file.open.cancel", "Cancel Opening Files",
    "process-stop-symbolic", "File", "Stop pending opens and retain completed documents")
UMI_STUDIO_GTK_COMMAND_PROFILE(
    "opening_results", "file.open.results", "Opening Results",
    "dialog-information-symbolic", "File", "Review outcomes from the latest completed selection")
