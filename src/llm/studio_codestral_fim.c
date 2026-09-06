/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/llm/studio_codestral_fim.c
 *
 * PURPOSE:
 *   Implement the studio codestral fim behavior for
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
 * File: src/llm/studio_codestral_fim.c
 * PURPOSE: Utilities for "Fill-In-the-Middle" (FIM) prompt shaping used by
 *          code models (e.g., Codestral).  Provides helpers to:
 *            - split a buffer at caret/cursor
 *            - build a FIM composite prompt with model-specific tokens
 *            - (optionally) strip placeholder marker near the split
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#include <glib.h>
#include <string.h>
#include <llm.h>

/* Small helpers read environment overrides; fall back to defaults. */
static const char *default_pre(void) {
  const char *v = g_getenv("UMI_FIM_PRE");
  return (v && *v) ? v : "<PRE>";
}
/* Provide the default mid operation used by this module and its client applications. */
static const char *default_mid(void) {
  const char *v = g_getenv("UMI_FIM_MID");
  return (v && *v) ? v : "<MID>";
}
/* Provide the default suf operation used by this module and its client applications. */
static const char *default_suf(void) {
  const char *v = g_getenv("UMI_FIM_SUF");
  return (v && *v) ? v : "<SUF>";
}

/* Build a FIM prompt: [pre][prompt][mid][suffix][suf] */
gchar *umi_fim_build_prompt(const char *prompt,
                            const char *suffix,
                            const char *pre_t,  /* optional override token */
                            const char *mid_t,  /* optional override token */
                            const char *suf_t)  /* optional override token */
{
  /* Resolve tokens with portable ternary, not GNU ':?'. */
  const char *P = (pre_t && *pre_t) ? pre_t : default_pre();
  const char *M = (mid_t && *mid_t) ? mid_t : default_mid();
  const char *S = (suf_t && *suf_t) ? suf_t : default_suf();

  const char *A = prompt ? prompt : "";
  const char *B = suffix ? suffix : "";

  /* Build through GLib's length-aware string buffer.  This keeps every append
   * bounded by the buffer implementation and avoids the overflow risk of
   * repeated strcat calls when an editor supplies a large suffix. */
  GString *builder = g_string_new(P);
  if (builder == NULL) return NULL;
  g_string_append(builder, A);
  g_string_append(builder, M);
  g_string_append(builder, B);
  g_string_append(builder, S);

  /* Transfer the finished allocation to the caller, who releases it with
   * g_free() as documented by this helper's public declaration. */
  return g_string_free(builder, FALSE);
}
