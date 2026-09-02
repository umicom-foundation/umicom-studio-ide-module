/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/dialogs/include/about.h
 *
 * PURPOSE:
 *   Declare the about contract shared by Framework services and thin
 *   applications.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/dialog/include/about.h
 *
 * PURPOSE:
 *   Public helper to present the GTK4 About dialog in a non-blocking way.
 *
 * API:
 *   void umi_about_show(GtkWindow *parent);
 *   void umi_show_about(GtkWindow *parent); // legacy alias
 *
 * Created by: Umicom Foundation | Developer: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_ABOUT_H
#define UMICOM_ABOUT_H

#include <gtk/gtk.h>

/**
 * Provide the about show operation used by this module and its client applications.
 */
void umi_about_show (GtkWindow *parent);
/**
 * Provide the show about operation used by this module and its client applications.
 */
void umi_show_about (GtkWindow *parent); /* legacy alias retained */

#endif /* UMICOM_ABOUT_H */
