/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/profiles/commands/reopen_document.inc
 * PURPOSE: Expose the shared document history in Studio command search.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

/* Presentation only: Framework owns paths, bounds and file loading. */
UMI_STUDIO_GTK_COMMAND_PROFILE("reopen_document", "file.reopen-closed",
    "Reopen Closed Document", "document-open-recent-symbolic", "File",
    "Open the latest closed saved path; discarded draft text is not restored")
UMI_STUDIO_GTK_COMMAND_PROFILE("forget_closed_document", "file.forget-closed",
    "Forget Last Closed Document", "edit-clear-symbolic", "File",
    "Skip the latest closed path without deleting its file")
