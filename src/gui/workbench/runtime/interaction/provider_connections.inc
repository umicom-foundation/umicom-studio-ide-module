/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/provider_connections.inc
 * PURPOSE: Open Framework-owned connection settings from Studio's AI surface.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/provider_connections/gtk4.h"

static void RuntimeProviderConnectionsClicked(GtkButton *button, gpointer data)
{
    GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(button));
    if (root == NULL || !GTK_IS_WINDOW(root)) return;
    /* Only application scope is selected here. Framework owns local paths,
     * workers and settings review. This desktop scope is not an authenticated
     * account, and opening it never changes Studio's active AI provider. */
    UmiStatus status = UmiProviderConnectionsGtkPresent(GTK_WINDOW(root), "studio", "desktop");
    gtk_label_set_text(GTK_LABEL(data), status == UMI_STATUS_OK ? ""
        : "Connection settings could not open. Check the local application data directory and available storage support.");
}
static void RuntimeProviderConnectionsControls(GtkWidget *box)
{
    GtkWidget *button = gtk_button_new_with_label("Connections…");
    GtkWidget *message = gtk_label_new("");
    gtk_label_set_wrap(GTK_LABEL(message), TRUE);
    gtk_label_set_xalign(GTK_LABEL(message), 0.0F);
    g_object_set_data_full(G_OBJECT(button), "umicom-automation-id", g_strdup("studio.ai.connections"), g_free);
    /* The signal follows the message widget's lifetime, not a raw Studio
     * runtime pointer. Retained controls cannot access a retired workbench. */
    g_signal_connect_object(button, "clicked", G_CALLBACK(RuntimeProviderConnectionsClicked), message, 0);
    gtk_box_append(GTK_BOX(box), button);
    gtk_box_append(GTK_BOX(box), message);
    GtkWidget *help = gtk_label_new("Select a saved connection in Connections, then choose Chat with selected saved connection to review and send one text request.");
    gtk_label_set_wrap(GTK_LABEL(help), TRUE);
    gtk_label_set_xalign(GTK_LABEL(help), 0.0F);
    gtk_box_append(GTK_BOX(box), help);
}
