/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_provider_connections_entry_gtk4.c
 * PURPOSE: Check the real thin settings entry without opening personal storage.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
/* Substitute only the window entry point. The real Studio callback and weak
 * signal binding remain under test; no window is presented or database opened. */
#define UmiProviderConnectionsGtkPresent FixturePresentConnections
#include "../src/gui/workbench/runtime/interaction/provider_connections.inc"
#undef UmiProviderConnectionsGtkPresent
static unsigned calls;
static UmiStatus reply;
static GtkWindow *expected_parent;
UmiStatus FixturePresentConnections(GtkWindow *parent, const char *application, const char *profile)
{
    ++calls;
    if (parent != expected_parent || strcmp(application, "studio") != 0 || strcmp(profile, "desktop") != 0)
        return UMI_STATUS_INVALID_ARGUMENT;
    return reply;
}
#define CHECK(value) do { if (!(value)) { fprintf(stderr, "line %d: %s\n", __LINE__, #value); failed = 1; goto cleanup; } } while (0)
int main(void)
{
    if (!gtk_init_check()) return 77;
    int failed = 0;
    GtkWidget *window = gtk_window_new(); g_object_ref(window);
    expected_parent = GTK_WINDOW(window);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_window_set_child(GTK_WINDOW(window), box);
    RuntimeProviderConnectionsControls(box);
    GtkWidget *button = gtk_widget_get_first_child(box);
    GtkWidget *message = gtk_widget_get_next_sibling(button);
    CHECK(GTK_IS_BUTTON(button) && GTK_IS_LABEL(message));
    CHECK(strcmp(g_object_get_data(G_OBJECT(button), "umicom-automation-id"), "studio.ai.connections") == 0);
    g_signal_emit_by_name(button, "clicked");
    CHECK(calls == 1U && gtk_label_get_text(GTK_LABEL(message))[0] == '\0');
    reply = UMI_STATUS_IO_ERROR;
    g_signal_emit_by_name(button, "clicked");
    CHECK(calls == 2U && strstr(gtk_label_get_text(GTK_LABEL(message)), "could not open") != NULL);
    /* Removing the callback target disconnects a retained button. It must not
     * dispatch with a dangling Studio runtime or error-label pointer. */
    gtk_box_remove(GTK_BOX(box), message);
    g_signal_emit_by_name(button, "clicked");
    CHECK(calls == 2U);
cleanup:
    gtk_window_destroy(GTK_WINDOW(window)); g_object_unref(window);
    return failed;
}
