/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/umi_log.c
 *
 * PURPOSE:
 *   Implement the umi log behavior for
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
 * File: src/core/umi_log.c
 * PURPOSE: Implementation for tiny logging shim (see umi_log.h)
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-08 | MIT
 *---------------------------------------------------------------------------*/
#include "umi_log.h"
#include <stdarg.h>
#include <glib.h>

static volatile gint s_level = UMI_LOG_INFO;

/* Provide the log set level operation used by this module and its client applications. */
void umi_log_set_level(int level) {
  /* Apply this branch only when its contract condition is satisfied. */
  if (level < UMI_LOG_DEBUG) level = UMI_LOG_DEBUG;
  /* Preserve the original failure result so the caller can respond to the correct cause. */
  if (level > UMI_LOG_ERROR) level = UMI_LOG_ERROR;
  g_atomic_int_set(&s_level, level);
}

/* Provide the log get level operation used by this module and its client applications. */
int umi_log_get_level(void) {
  return g_atomic_int_get(&s_level);
}

/* Provide the to glib operation used by this module and its client applications. */
static GLogLevelFlags to_glib(int lvl){
  /* Select the behaviour associated with the requested command or state value. */
  switch (lvl){
    case UMI_LOG_DEBUG: return G_LOG_LEVEL_DEBUG;
    case UMI_LOG_INFO:  return G_LOG_LEVEL_MESSAGE;
    case UMI_LOG_WARN:  return G_LOG_LEVEL_WARNING;
    default:            return G_LOG_LEVEL_CRITICAL;
  }
}

/* Provide the logv operation used by this module and its client applications. */
void umi_logv(int level, const char *fmt, va_list ap){
  /* Apply this branch only when its contract condition is satisfied. */
  if (level < umi_log_get_level()) return;
  gchar *msg = g_strdup_vprintf(fmt, ap);
  g_log("umicom", to_glib(level), "%s", msg);
  g_free(msg);
}

/* Provide the log operation used by this module and its client applications. */
void umi_log(int level, const char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  umi_logv(level, fmt, ap);
  va_end(ap);
}
