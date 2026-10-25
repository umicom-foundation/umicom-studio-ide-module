/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/editor_whitespace.inc
 *
 * PURPOSE:
 *   Compose the Framework whitespace catalogue into one searchable Studio action menu.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/document/whitespace_commands.h"
static void RuntimeAppendWhitespaceActions(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *bar) {
  GtkWidget *menu = NULL;
  if (UmiGtk4ActionMenuCreate("Whitespace", &menu) != UMI_STATUS_OK)
    return;
  (void)umi_gtk4_automation_tag_widget(menu, "studio.editor.whitespace");
  gtk_box_append(GTK_BOX(bar), menu);
  for (size_t i = 0U; i < UmiDocumentWhitespaceCommandCount(); ++i) {
    const UmiDocumentWhitespaceCommand *command = UmiDocumentWhitespaceCommandAt(i);
    GtkWidget *button =
        runtime_make_command_button(runtime, command->label, command->id, command->keywords);
    gchar *tag = g_strdup_printf("studio.%s", command->id);
    (void)umi_gtk4_automation_tag_widget(button, tag);
    g_free(tag);
    UmiStatus status = UmiGtk4ActionMenuAppend(menu, button, command->keywords);
    /* A refused menu adoption must leave a working action reachable. */
    if (status != UMI_STATUS_OK && gtk_widget_get_parent(button) == NULL)
      gtk_box_append(GTK_BOX(bar), button);
  }
}
