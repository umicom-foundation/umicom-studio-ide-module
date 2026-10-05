/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/profiles/commands/navigation_history.inc
 * PURPOSE: Present shared location history in Studio command search.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

UMI_STUDIO_GTK_COMMAND_PROFILE("navigation_back", "navigate.back",
    "Navigate Back", "go-previous-symbolic", "Navigate", "Return to the previous visited source location")
UMI_STUDIO_GTK_COMMAND_PROFILE("navigation_forward", "navigate.forward",
    "Navigate Forward", "go-next-symbolic", "Navigate", "Return to the next visited source location")
UMI_STUDIO_GTK_COMMAND_PROFILE("navigation_clear", "navigate.history.clear",
    "Clear Navigation History", "edit-clear-symbolic", "Navigate", "Forget visited locations without closing or changing source documents")
