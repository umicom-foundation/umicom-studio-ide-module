/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/profiles/commands/bookmarks.inc
 * PURPOSE: Expose session source bookmarks through command search.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

UMI_STUDIO_GTK_COMMAND_PROFILE("bookmark_toggle", "editor.bookmark.toggle",
    "Bookmark Line", "bookmark-new-symbolic", "Navigate", "Add or remove a bookmark on the active line")
UMI_STUDIO_GTK_COMMAND_PROFILE("bookmark_next", "editor.bookmark.next",
    "Next Bookmark", "go-next-symbolic", "Navigate", "Visit the next available bookmarked line")
UMI_STUDIO_GTK_COMMAND_PROFILE("bookmark_previous", "editor.bookmark.previous",
    "Previous Bookmark", "go-previous-symbolic", "Navigate", "Visit the previous available bookmarked line")
UMI_STUDIO_GTK_COMMAND_PROFILE("bookmark_clear", "editor.bookmark.clear",
    "Clear Bookmarks", "edit-clear-symbolic", "Navigate", "Forget session bookmark locations without changing documents")
