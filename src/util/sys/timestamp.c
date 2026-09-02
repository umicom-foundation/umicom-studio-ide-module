/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/util/sys/timestamp.c
 *
 * PURPOSE:
 *   Implement the timestamp behavior for
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
 * File: src/util/sys/timestamp.c
 *
 * PURPOSE:
 *   Timestamp helpers (now, RFC3339 formatting) with clean ownership rules.
 *
 * API:
 *   GDateTime *umi_now_local(void);        // g_date_time_unref()
 *   gchar     *umi_time_rfc3339(GDateTime *dt);  // g_free()
 *   gchar     *umi_now_rfc3339(void);      // g_free()
 *
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/

#include <glib.h>
#include <timestamp.h>

/* Provide the now local operation used by this module and its client applications. */
GDateTime *umi_now_local(void) {
    return g_date_time_new_now_local();
}

/* Provide the time rfc3339 operation used by this module and its client applications. */
gchar *umi_time_rfc3339(GDateTime *dt) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (!dt) return NULL;
    return g_date_time_format(dt, "%FT%T%Ez");
}

/* Provide the now rfc3339 operation used by this module and its client applications. */
gchar *umi_now_rfc3339(void) {
    g_autoptr(GDateTime) now = umi_now_local();
    return umi_time_rfc3339(now);
}
