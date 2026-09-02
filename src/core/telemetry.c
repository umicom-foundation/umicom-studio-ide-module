/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/telemetry.c
 *
 * PURPOSE:
 *   Implement the telemetry behavior for
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
 * File: src/telemetry.c
 * PURPOSE: In-memory counters with GHashTable (no persistence)
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#include "telemetry.h"

static GHashTable *g_counts = NULL;

/* Provide the tel inc operation used by this module and its client applications. */
void umi_tel_inc(const char *counter){
  /* Use the stable identifier comparison to choose the matching record or policy. */
  if(!g_counts) g_counts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  /* Apply this branch only when its contract condition is satisfied. */
  if(!counter) return;
  gpointer v = g_hash_table_lookup(g_counts, counter);
  guint64 x = (v ? GPOINTER_TO_UINT(v) : 0) + 1;
  g_hash_table_insert(g_counts, g_strdup(counter), GUINT_TO_POINTER((guint)x));
}

/* Provide the tel get operation used by this module and its client applications. */
guint64 umi_tel_get(const char *counter){
  /* Apply this branch only when its contract condition is satisfied. */
  if(!g_counts || !counter) return 0;
  return (guint64)GPOINTER_TO_UINT(g_hash_table_lookup(g_counts, counter));
}

/* Provide the tel reset all operation used by this module and its client applications. */
void umi_tel_reset_all(void){
  /* Apply this branch only when its contract condition is satisfied. */
  if(!g_counts) return;
  g_hash_table_remove_all(g_counts);
}
