/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/util/sys/utf8.c
 *
 * PURPOSE:
 *   Implement the utf8 behavior for
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
 * File: src/util/sys/utf8.c
 *
 * PURPOSE:
 *   UTF-8 helpers (case-insensitive substring match) using GLib case folding.
 *
 * API:
 *   gboolean umi_utf8_icontains(const char *haystack, const char *needle);
 *
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/

#include <glib.h>
#include <utf8.h>

/* Provide the utf8 icontains operation used by this module and its client applications. */
gboolean umi_utf8_icontains(const char *haystack, const char *needle) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (!haystack || !needle) return FALSE;
    /* Apply this branch only when its contract condition is satisfied. */
    if (*needle == '\0') return TRUE;

    g_autofree gchar *h = g_utf8_casefold(haystack, -1);
    g_autofree gchar *n = g_utf8_casefold(needle,   -1);
    return g_strstr_len(h, -1, n) != NULL;
}
