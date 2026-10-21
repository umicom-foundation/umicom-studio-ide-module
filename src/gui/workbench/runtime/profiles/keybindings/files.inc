/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/profiles/keybindings/files.inc
 * PURPOSE: Bind file creation, selection and Save As to shared document actions.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
/* The runtime routes these commands through the existing Framework document
 * owner and native chooser. Shortcuts never write or select paths themselves. */
UMI_STUDIO_GTK_KEY_PROFILE(GDK_KEY_n, GDK_KEY_N, GDK_CONTROL_MASK,
    "file.new", "New File")
UMI_STUDIO_GTK_KEY_PROFILE(GDK_KEY_o, GDK_KEY_O, GDK_CONTROL_MASK,
    "file.open", "Open File")
UMI_STUDIO_GTK_KEY_PROFILE(GDK_KEY_s, GDK_KEY_S, (GDK_CONTROL_MASK | GDK_SHIFT_MASK),
    "file.save-as", "Save As")

/* Search navigation is a local editor action; matching stays in Framework. */
UMI_STUDIO_GTK_KEY_PROFILE(GDK_KEY_f, GDK_KEY_F, GDK_CONTROL_MASK,
    "studio.editor.find", "Find in Active Document")

UMI_STUDIO_GTK_KEY_PROFILE(GDK_KEY_h, GDK_KEY_H, GDK_CONTROL_MASK,
    "studio.editor.replace", "Replace in Active Document")
