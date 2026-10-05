/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/keybindings/document_tabs.inc
 * PURPOSE: Bind source-document navigation to Framework's shared command route.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_KEY_PROFILE(GDK_KEY_Tab, GDK_KEY_Tab, GDK_CONTROL_MASK,
    "navigate.document.next", "Next Source Document")
UMI_STUDIO_GTK_KEY_PROFILE(GDK_KEY_ISO_Left_Tab, GDK_KEY_Tab,
    (GDK_CONTROL_MASK | GDK_SHIFT_MASK),
    "navigate.document.previous", "Previous Source Document")

/* This shortcut works after the last source tab closes as well. */
UMI_STUDIO_GTK_KEY_PROFILE(GDK_KEY_t, GDK_KEY_T,
    (GDK_CONTROL_MASK | GDK_SHIFT_MASK),
    "file.reopen-closed", "Reopen Closed Document")

/* History navigation restores source locations; tab cycling remains Ctrl+Tab. */
UMI_STUDIO_GTK_KEY_PROFILE(GDK_KEY_Left, GDK_KEY_Left, GDK_ALT_MASK,
    "navigate.back", "Navigate Back")
UMI_STUDIO_GTK_KEY_PROFILE(GDK_KEY_Right, GDK_KEY_Right, GDK_ALT_MASK,
    "navigate.forward", "Navigate Forward")
