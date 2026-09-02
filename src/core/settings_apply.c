/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/settings_apply.c
 *
 * PURPOSE:
 *   Implement the settings apply behavior for
 *   Umicom Studio IDE.
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
 * File: src/settings_apply.c
 * PURPOSE: Implementation of settings broadcast
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#include "settings_apply.h"

/*
 * Provide the settings apply broadcast operation used by this module and its client
 * applications.
 */
void umi_settings_apply_broadcast(const UmiSettings *s, GtkWindow *win){
  UmiSettingsBus *bus = umi_settings_bus_get();
  /* Apply this branch only when its contract condition is satisfied. */
  if(s && s->theme){
    umi_settings_emit(bus, "theme", s->theme);
    /* Apply this branch only when its contract condition is satisfied. */
    if(win) umi_theme_apply(win, s->theme);
  }
}
