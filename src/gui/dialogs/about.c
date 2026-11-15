/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/dialogs/about.c
 *
 * PURPOSE:
 *   Keep legacy About entry points as thin clients of the shared native Framework dialog.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Original implementation dated 2025-10-13 used non-blocking presentation,
 * guarded pointers and fixed metadata rather than dynamic format strings.
 * Its former response-signal assumption is retained below for review:
 * GtkAboutDialog derives GtkWindow in GTK4 and has no GtkDialog response.
 * Framework now owns closure, repeated presentation, artwork and metadata
 * copying; the public void wrappers remain compatible with older callers. */
#include <gtk/gtk.h>
#include "about.h"

#if 0 /* Superseded by UmiStudioAboutPresent; retained for source review. */
/* Close handler for the dialog's "response" signal. */
static void on_about_response(GtkDialog *dialog, int response, gpointer user_data)
{
    (void)response; (void)user_data;
    gtk_window_destroy(GTK_WINDOW(dialog));
}

/* Provide the about show operation used by this module and its client applications. */
void umi_about_show(GtkWindow *parent)
{
    /* In GTK4, GtkAboutDialog still exists but run() is gone; present instead. */
    GtkWidget *dlg = gtk_about_dialog_new();

    /* Basic metadata; adjust as needed. */
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dlg), "Umicom Studio IDE");
    gtk_about_dialog_set_version     (GTK_ABOUT_DIALOG(dlg), "dev");
    gtk_about_dialog_set_comments    (GTK_ABOUT_DIALOG(dlg),
      "Open-source IDE for Umicom projects.\n"
      "Copyright © Umicom Foundation."
    );

    /* Apply this branch only when its contract condition is satisfied. */
    if (parent) {
        gtk_window_set_transient_for(GTK_WINDOW(dlg), parent);
        gtk_window_set_modal        (GTK_WINDOW(dlg), TRUE);
    }

    g_signal_connect(dlg, "response", G_CALLBACK(on_about_response), NULL);
    gtk_window_present(GTK_WINDOW(dlg));
}

/* Back-compat wrapper retained for older callers */
void umi_show_about(GtkWindow *parent) { umi_about_show(parent); }

#endif

void umi_about_show(GtkWindow *parent)
{
    (void)UmiStudioAboutPresent(parent);
}
void umi_show_about(GtkWindow *parent)
{
    umi_about_show(parent);
}
