/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/actions.c
 *
 * PURPOSE:
 *   Implement the actions behavior for
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
 * File: src/actions.c
 * PURPOSE: Implementation of default actions (no-op callbacks)
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#include "actions.h"

/* Provide the noop operation used by this module and its client applications. */
static void noop(GSimpleAction *a, GVariant *p, gpointer u){ (void)a;(void)p;(void)u; }

/* Provide the actions install operation used by this module and its client applications. */
void umi_actions_install(GtkApplication *app){
  /* Apply this branch only when its contract condition is satisfied. */
  if(!app) return;
  const char *names[] = {
    "open-folder", "save", "run", "stop", "palette", "preferences", "quit", NULL
  };
  /* Visit each bounded item once so every record receives the same rule. */
  for(const char **n=names; *n; ++n){
    GSimpleAction *act = g_simple_action_new(*n, NULL);
    g_signal_connect(act, "activate", G_CALLBACK(noop), NULL);
    g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
  }
}
