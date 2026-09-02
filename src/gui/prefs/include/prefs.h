/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/prefs/include/prefs.h
 *
 * PURPOSE:
 *   Declare the prefs contract shared by Framework services and thin
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
 * File: src/gui/prefs/include/prefs.h
 *
 * PURPOSE:
 *   Higher-level Preferences API (settings model + modal helpers). This is
 *   optional if you use prefs_ui.h directly; provided here for completeness.
 *
 * DESIGN:
 *   - Small POD settings struct with loader/saver helpers.
 *   - UI helper functions for modal presentation.
 *
 * Created by: Umicom Foundation | Developer: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/
#ifndef UMI_PREFS_H
#define UMI_PREFS_H

#include <glib.h>
#include <gtk/gtk.h>

/**
 * Represent the settings data shared with callers of this public contract.
 */
typedef struct UmiSettings {
  char    *theme;                 /* theme name or NULL for default             */
  int      font_size;             /* editor font size in points                 */
  gboolean autosave_enabled;      /* optional future field                      */
  int      autosave_interval_sec; /* optional future field                      */
} UmiSettings;

/**
 * Read settings into validated module state and return a status when input cannot be used.
 */
UmiSettings *umi_settings_load(void);
/**
 * Write settings in its stable representation and report capacity or input failures to the
 * caller.
 */
gboolean     umi_settings_save(const UmiSettings *s);
/**
 * Provide the settings free operation used by this module and its client applications.
 */
void         umi_settings_free(UmiSettings *s);

/* Optional controller wrapper (separate from prefs_ui.h controller).         */
typedef struct UmiPrefs UmiPrefs;

/**
 * Provide the prefs new operation used by this module and its client applications.
 */
UmiPrefs  *umi_prefs_new(void);
/**
 * Provide the prefs free operation used by this module and its client applications.
 */
void       umi_prefs_free(UmiPrefs *p);
/**
 * Provide the prefs set parent operation used by this module and its client applications.
 */
void       umi_prefs_set_parent(UmiPrefs *p, GtkWindow *parent);
/**
 * Provide the prefs widget operation used by this module and its client applications.
 */
GtkWidget *umi_prefs_widget(UmiPrefs *p);
/**
 * Provide the prefs show modal operation used by this module and its client applications.
 */
gboolean   umi_prefs_show_modal(UmiPrefs *p, GtkWindow *parent);

#endif /* UMI_PREFS_H */
