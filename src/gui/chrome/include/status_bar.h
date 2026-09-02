/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/chrome/include/status_bar.h
 *
 * PURPOSE:
 *   Declare the status bar contract shared by Framework services and thin
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
 * File: src/gui/chrome/include/status_bar.h
 *
 * PURPOSE:
 *   Public API for a minimal, safe status bar used by the chrome module.
 *   The implementation lives in src/gui/chrome/status_bar.c.
 *
 * DESIGN:
 *   - Opaque handle (UmiStatusBar) with a single visible GtkWidget*.
 *   - GTK4-only (GtkBox + GtkLabel internally).
 *   - Thread-agnostic: all calls should be made from the GTK main thread.
 *
 * API:
 *   typedef struct _UmiStatusBar UmiStatusBar;
 *   UmiStatusBar *umi_status_bar_new(void);
 *   GtkWidget    *umi_status_bar_widget(UmiStatusBar *sb);   // pack this
 *   void          umi_status_bar_set  (UmiStatusBar *sb, const char *text);
 *   void          umi_status_bar_flash(UmiStatusBar *sb, const char *text, guint ms);
 *   void          umi_status_bar_free (UmiStatusBar *sb);
 *
 * Created by: Umicom Foundation | Developer: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/
#ifndef USIDE_STATUS_BAR_H
#define USIDE_STATUS_BAR_H

#include <gtk/gtk.h>
#include <glib.h>

G_BEGIN_DECLS

/**
 * Represent the status bar data shared with callers of this public contract.
 */
typedef struct _UmiStatusBar UmiStatusBar;

/**
 * Provide the status bar new operation used by this module and its client applications.
 */
UmiStatusBar *umi_status_bar_new(void);
/**
 * Provide the status bar widget operation used by this module and its client applications.
 */
GtkWidget    *umi_status_bar_widget(UmiStatusBar *sb);
/**
 * Copy status bar into module-owned storage so callers keep ownership of their input
 * values.
 */
void          umi_status_bar_set  (UmiStatusBar *sb, const char *text);
/**
 * Provide the status bar flash operation used by this module and its client applications.
 */
void          umi_status_bar_flash(UmiStatusBar *sb, const char *text, guint ms);
/**
 * Provide the status bar free operation used by this module and its client applications.
 */
void          umi_status_bar_free (UmiStatusBar *sb);

G_END_DECLS

#endif /* USIDE_STATUS_BAR_H */
