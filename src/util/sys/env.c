/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/util/sys/env.c
 *
 * PURPOSE:
 *   Implement the env behavior for
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
 * File: src/util/sys/env.c
 *
 * PURPOSE:
 *   Small helpers around environment variables (get/set with defaults).
 *
 * API:
 *   gchar   *umi_env_get(const char *key);                 // g_free()
 *   gchar   *umi_env_get_or(const char *key, const char *fallback); // g_free()
 *   gboolean umi_env_set(const char *key, const char *value, gboolean overwrite);
 *
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/

#include <glib.h>
#include <env.h>

/* Provide the env get operation used by this module and its client applications. */
gchar *umi_env_get(const char *key) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (!key) return NULL;
    const char *v = g_getenv(key);
    return v ? g_strdup(v) : NULL;
}

/* Provide the env get or operation used by this module and its client applications. */
gchar *umi_env_get_or(const char *key, const char *fallback) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (!key) return fallback ? g_strdup(fallback) : NULL;
    const char *v = g_getenv(key);
    return g_strdup(v ? v : fallback);
}

/* Copy env into module-owned storage so callers keep ownership of their input values. */
gboolean umi_env_set(const char *key, const char *value, gboolean overwrite) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (!key) return FALSE;
    return g_setenv(key, value ? value : "", overwrite);
}
