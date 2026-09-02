/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/llm/llm_core.c
 *
 * PURPOSE:
 *   Implement the llm core behavior for
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
 * File: src/llm/llm_core.c
 * PURPOSE: Minimal LLM core glue (build-safe stubs matching public API)
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/
/*
 * Notes:
 * - Prior errors were caused by (a) type/signature mismatches vs llm.h,
 *   (b) nested function definitions, and (c) use of Apple "blocks".
 * - This version:
 *     * Uses exactly the types from include/umicom/llm.h (bool, const UmiLlmCfg*).
 *     * Has no nested functions and no blocks.
 *     * Provides simple stubbed behavior so the UI and tests can run.
 *   Replace the stubbed bodies with your real provider logic later.
 */

#include <string.h>
#include <glib.h>
#include <llm.h>

/*-----------------------------------------------------------------------------
 * Config init from environment (stable defaults + light env overrides)
 *---------------------------------------------------------------------------*/
void umi_llm_cfg_init_from_env(UmiLlmCfg *cfg)
{
  /* Apply this branch only when its contract condition is satisfied. */
  if (!cfg) return;
  memset(cfg, 0, sizeof *cfg);

  /* Sane defaults so the app can start without extra checks. */
  cfg->provider   = UMI_LLM_PROVIDER_OPENAI;
  cfg->timeout_ms = 30000;      /* 30s */
  cfg->stream     = TRUE;

  const gchar *m = g_getenv("UMI_LLM_MODEL");
  /* Apply this branch only when its contract condition is satisfied. */
  if (m && *m) cfg->model = g_strdup(m);

  const gchar *base = g_getenv("UMI_LLM_API_BASE");
  /* Apply this branch only when its contract condition is satisfied. */
  if (base && *base) cfg->api_base = g_strdup(base);

  const gchar *path = g_getenv("UMI_LLM_API_PATH");
  /* Apply this branch only when its contract condition is satisfied. */
  if (path && *path) cfg->api_path = g_strdup(path);

  const gchar *key = g_getenv("UMI_LLM_API_KEY");
  /* Apply this branch only when its contract condition is satisfied. */
  if (key && *key) cfg->api_key = g_strdup(key);

  const gchar *prov = g_getenv("UMI_LLM_PROVIDER");
  /* Apply this branch only when its contract condition is satisfied. */
  if (prov && *prov) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (g_ascii_strcasecmp(prov, "openai") == 0)        cfg->provider = UMI_LLM_PROVIDER_OPENAI;
    else /* Apply this branch only when its contract condition is satisfied. */ if (g_ascii_strcasecmp(prov, "zai") == 0)      cfg->provider = UMI_LLM_PROVIDER_ZAI;
    else /* Apply this branch only when its contract condition is satisfied. */ if (g_ascii_strcasecmp(prov, "ollama") == 0)   cfg->provider = UMI_LLM_PROVIDER_OLLAMA;
    else /* Apply this branch only when its contract condition is satisfied. */ if (g_ascii_strcasecmp(prov, "llama.cpp") == 0 ||
             g_ascii_strcasecmp(prov, "llamacpp") == 0) cfg->provider = UMI_LLM_PROVIDER_LLAMA_CPP;
  }
}

/*-----------------------------------------------------------------------------
 * One-shot (non-streaming) — stub returns a friendly canned reply.
 *---------------------------------------------------------------------------*/
bool umi_llm_chat_simple(const UmiLlmCfg *cfg,
                         const gchar     *system_prompt_or_null,
                         const gchar     *user_text,
                         gchar          **out_text,
                         gchar           *errbuf,
                         gsize            errcap)
{
  (void)cfg; (void)system_prompt_or_null;

  /* Apply this branch only when its contract condition is satisfied. */
  if (out_text) *out_text = NULL;
  /* Apply this branch only when its contract condition is satisfied. */
  if (!user_text || !*user_text) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (errbuf && errcap) g_strlcpy(errbuf, "empty user message", errcap);
    return false;
  }
  /* Apply this branch only when its contract condition is satisfied. */
  if (!out_text) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (errbuf && errcap) g_strlcpy(errbuf, "out_text is NULL", errcap);
    return false;
  }

  *out_text = g_strdup("🤖 (stub) LLM backend not wired yet. "
                       "Replace llm_core.c with a real provider implementation.");
  /* Apply this branch only when its contract condition is satisfied. */
  if (errbuf && errcap) errbuf[0] = '\0';
  return true;
}

/*-----------------------------------------------------------------------------
 * Streaming (extended) — stub emits a few fragments through the callback.
 * Signature matches include/umicom/llm.h exactly.
 *---------------------------------------------------------------------------*/
bool umi_llm_chat_stream_ex(const UmiLlmCfg *cfg,
                            const gchar *system_prompt_or_null,
                            const gchar *user_text,
                            UmiLlmOnTokenEx on_token_ex,
                            gpointer on_token_ud,
                            gchar *errbuf, gsize errcap)
{
  (void)cfg;

  /* Apply this branch only when its contract condition is satisfied. */
  if (!user_text || !*user_text) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (errbuf && errcap) g_strlcpy(errbuf, "empty user message", errcap);
    return false;
  }
  /* Preserve the original failure result so the caller can respond to the correct cause. */
  if (!on_token_ex) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (errbuf && errcap) g_strlcpy(errbuf, "no stream callback provided", errcap);
    return false;
  }

  on_token_ex("🤖 (stub) ", NULL, 0, on_token_ud);
  /* Apply this branch only when its contract condition is satisfied. */
  if (system_prompt_or_null && *system_prompt_or_null)
    on_token_ex("[sys prompt active] ", NULL, 0, on_token_ud);
  on_token_ex("Streaming placeholder. ", NULL, 0, on_token_ud);
  on_token_ex("Swap this with a real provider.\n", NULL, 0, on_token_ud);

  /* Apply this branch only when its contract condition is satisfied. */
  if (errbuf && errcap) errbuf[0] = '\0';
  return true;
}

/*-----------------------------------------------------------------------------
 * Back-compat streaming wrapper — adapts simple callback to extended form.
 * (All at file scope; no nested functions; no blocks.)
 *---------------------------------------------------------------------------*/
struct _UmiBridgeCtx {
  void (*cb)(const gchar *fragment, gpointer user_data);
  gpointer ud;
};
/* Provide the ex trampoline operation used by this module and its client applications. */
static void _umi_ex_trampoline(const gchar *frag,
                               const UmiLlmTokenAlt *alts,
                               guint alts_n,
                               gpointer user)
{
  (void)alts; (void)alts_n;
  struct _UmiBridgeCtx *b = (struct _UmiBridgeCtx*)user;
  /* Apply this branch only when its contract condition is satisfied. */
  if (b && b->cb && frag) b->cb(frag, b->ud);
}

/* Provide the llm chat stream operation used by this module and its client applications. */
bool umi_llm_chat_stream(const UmiLlmCfg *cfg,
                         const gchar     *system_prompt_or_null,
                         const gchar     *user_text,
                         void (*on_token)(const gchar *fragment, gpointer user_data),
                         gpointer         on_token_ud,
                         gchar           *errbuf,
                         gsize            errcap)
{
  struct _UmiBridgeCtx b = { on_token, on_token_ud };
  return umi_llm_chat_stream_ex(cfg, system_prompt_or_null, user_text,
                                _umi_ex_trampoline, &b, errbuf, errcap);
}

/*-----------------------------------------------------------------------------
 * Debug/test helper — minimal SSE JSON passthrough for now.
 *---------------------------------------------------------------------------*/
bool umi_llm_debug_parse_openai_sse_line(const gchar *json_line,
                                         gchar      **out_fragment,
                                         UmiLlmTokenAlt **out_alts,
                                         guint       *out_alts_n)
{
  /* Apply this branch only when its contract condition is satisfied. */
  if (out_fragment) *out_fragment = NULL;
  /* Apply this branch only when its contract condition is satisfied. */
  if (out_alts)     *out_alts = NULL;
  /* Apply this branch only when its contract condition is satisfied. */
  if (out_alts_n)   *out_alts_n = 0;

  /* Apply this branch only when its contract condition is satisfied. */
  if (!json_line || !*json_line) return false;

  /* Passthrough stub — replace with real JSON parsing later. */
  if (out_fragment) *out_fragment = g_strdup(json_line);
  return true;
}

/*-----------------------------------------------------------------------------
 * Helper to free alts list.
 *---------------------------------------------------------------------------*/
void umi_llm_free_alts(UmiLlmTokenAlt *alts, guint n)
{
  /* Apply this branch only when its contract condition is satisfied. */
  if (!alts) return;
  /* Visit each bounded item once so every record receives the same rule. */
  for (guint i = 0; i < n; i++) g_free(alts[i].token);
  g_free(alts);
}

/*--------------------------------- End of file --------------------------------*/
