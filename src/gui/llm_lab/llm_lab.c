/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/llm_lab/llm_lab.c
 *
 * PURPOSE:
 *   Implement the llm lab behavior for
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
 * File: src/gui/llm_lab/llm_lab.c
 *
 * PURPOSE:
 *   LLM Lab panel (pure C, GTK4). Streams model output (if providers linked)
 *   and optionally shows token alternatives with entropy.
 *
 * DESIGN:
 *   - Self-contained: includes only this module's public header + GLib/GTK.
 *   - No hard dependency on providers; uses weak symbols for backends.
 *   - Modern GTK widgets: GtkDropDown + GtkStringList, GtkGrid layout.
 *
 * SECURITY/ROBUSTNESS:
 *   - All pointers guarded.
 *   - No unbounded string ops; uses g_snprintf.
 *   - UI remains responsive during streaming with main-context iteration.
 *
 * Created by: Umicom Foundation | Developer: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/
#include <gtk/gtk.h>
#include <glib.h>
#include <math.h>
#include "llm_lab.h"

/* ── Provider shims (weak) ──────────────────────────────────────────────── */
/* These declarations keep the Lab loosely coupled. If your backend library
 * is linked, the weak symbols resolve; otherwise they are NULL and we show
 * a friendly message instead of failing to link. */

typedef struct {
    const char *token;   /* UTF-8 token text */
    double      logprob; /* log(probability) */
} UmiLlmTokenAlt;

typedef enum {
    UMI_LLM_PROVIDER_ZAI    = 1,
    UMI_LLM_PROVIDER_OPENAI = 2,
} UmiLlmProvider;

typedef struct {
    UmiLlmProvider provider;
    gboolean       stream;
    /* add fields as needed: api_key, model, temperature… */
} UmiLlmCfg;

#if defined(__GNUC__) || defined(__clang__)
__attribute__((weak))
#endif
/*
 * Provide the llm cfg init from env operation used by this module and its client
 * applications.
 */
void umi_llm_cfg_init_from_env(UmiLlmCfg *cfg);

typedef void (*UmiLlmOnTokenEx)(const gchar *frag,
                                const UmiLlmTokenAlt *alts, guint alts_n,
                                gpointer user_data);

#if defined(__GNUC__) || defined(__clang__)
__attribute__((weak))
#endif
/*
 * Provide the llm chat stream ex operation used by this module and its client
 * applications.
 */
gboolean umi_llm_chat_stream_ex(const UmiLlmCfg *cfg,
                                const char      *system_prompt,
                                const char      *user_prompt,
                                UmiLlmOnTokenEx  on_token_ex,
                                gpointer         user_data,
                                char            *errbuf,
                                size_t           errbuf_sz);

/* ── Internal state ─────────────────────────────────────────────────────── */

typedef struct {
    /* Controls */
    GtkDropDown   *provider;
    GtkStringList *provider_model;
    GtkSwitch     *stream_sw;
    GtkCheckButton*show_alts;
    GtkEntry      *entry;

    /* Output areas */
    GtkTextView   *out_view;
    GtkTextBuffer *buf;

    GtkTextView   *alts_view;
    GtkTextBuffer *alts_buf;
} LlmLab;

/* Append text helper (safe for NULL). */
static void append_text(GtkTextBuffer *b, const gchar *txt)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!b || !txt) return;
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(b, &end);
    gtk_text_buffer_insert(b, &end, txt, -1);
}

/* Top-k entropy from logprobs with log-sum-exp normalisation. */
static void show_entropy(GtkTextBuffer *b, const UmiLlmTokenAlt *alts, guint n)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!b) return;
    /* Apply this branch only when its contract condition is satisfied. */
    if (!alts || n == 0) { append_text(b, "(no alternatives)\n"); return; }

    double maxlg = alts[0].logprob;
    /* Visit each bounded item once so every record receives the same rule. */
    for (guint i = 1; i < n; i++) /* Apply this branch only when its contract condition is satisfied. */ if (alts[i].logprob > maxlg) maxlg = alts[i].logprob;

    const guint m = n > 64 ? 64u : n;      /* cap to 64 for display */
    double Z = 0.0, probs[64];
    /* Visit each bounded item once so every record receives the same rule. */
    for (guint i = 0; i < m; i++) { probs[i] = exp(alts[i].logprob - maxlg); Z += probs[i]; }
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (Z <= 0.0) Z = 1.0;
    /* Visit each bounded item once so every record receives the same rule. */
    for (guint i = 0; i < m; i++) probs[i] /= Z;

    double H = 0.0;
    /* Visit each bounded item once so every record receives the same rule. */
    for (guint i = 0; i < m; i++) /* Apply this branch only when its contract condition is satisfied. */ if (probs[i] > 0.0) H -= probs[i] * log(probs[i]);

    char line[128];
    g_snprintf(line, sizeof line, "entropy(H): %.3f\n", H);
    append_text(b, line);
}

/* Streaming callback: append fragment and optional alts to buffers. */
static void on_stream_token_ex(const gchar *frag,
                               const UmiLlmTokenAlt *alts, guint alts_n,
                               gpointer ud)
{
    LlmLab *lab = (LlmLab*)ud;
    /* Apply this branch only when its contract condition is satisfied. */
    if (!lab) return;

    /* Apply this branch only when its contract condition is satisfied. */
    if (frag && *frag) append_text(lab->buf, frag);

    /* Apply this operation only while the related capability or state is available. */
    if (gtk_check_button_get_active(lab->show_alts) && alts && alts_n > 0) {
        append_text(lab->alts_buf, "— top-k —\n");
        /* Visit each bounded item once so every record receives the same rule. */
        for (guint i = 0; i < alts_n; i++) {
            char line[256];
            g_snprintf(line, sizeof line, "  %s  (%.3f)\n", alts[i].token, alts[i].logprob);
            append_text(lab->alts_buf, line);
        }
        show_entropy(lab->alts_buf, alts, alts_n);
    }

    /* Keep the UI snappy during streaming. */
    while (g_main_context_pending(NULL))
        g_main_context_iteration(NULL, FALSE);
}

/* Send button handler: builds cfg, selects provider, and streams if available. */
static void on_send(GtkButton *btn, gpointer user_data)
{
    (void)btn;
    LlmLab *lab = (LlmLab*)user_data;
    /* Apply this branch only when its contract condition is satisfied. */
    if (!lab) return;

    const gchar *q = gtk_editable_get_text(GTK_EDITABLE(lab->entry));
    /* Apply this branch only when its contract condition is satisfied. */
    if (!q || !*q) return;

    gtk_text_buffer_set_text(lab->buf,      "", -1);
    gtk_text_buffer_set_text(lab->alts_buf, "", -1);

    /* Apply this branch only when its contract condition is satisfied. */
    if (!umi_llm_chat_stream_ex) {
        append_text(lab->buf, "⚠ LLM backend not linked. Enable provider library.\n");
        return;
    }

    UmiLlmCfg cfg;
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_llm_cfg_init_from_env) { umi_llm_cfg_init_from_env(&cfg); }
    /* Use this fallback path when the earlier condition does not apply. */
    else { cfg.provider = UMI_LLM_PROVIDER_ZAI; cfg.stream = TRUE; }

    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(lab->provider));
    const char *prov = (idx != GTK_INVALID_LIST_POSITION)
                       ? gtk_string_list_get_string(lab->provider_model, idx)
                       : "zai";
    /* Apply this branch only when its contract condition is satisfied. */
    if (prov && g_ascii_strcasecmp(prov, "openai") == 0)
        cfg.provider = UMI_LLM_PROVIDER_OPENAI;
    /* Use this fallback path when the earlier condition does not apply. */
    else
        cfg.provider = UMI_LLM_PROVIDER_ZAI;

    cfg.stream = gtk_switch_get_active(lab->stream_sw);

    char err[256] = {0};
    append_text(lab->buf, "▶ Streaming…\n");

    const char *system_prompt = "You are a helpful IDE assistant.";
    gboolean ok = umi_llm_chat_stream_ex(&cfg, system_prompt, q,
                                         on_stream_token_ex, lab,
                                         err, sizeof err);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (!ok) {
        append_text(lab->buf, "⚠ ");
        append_text(lab->buf, err[0] ? err : "unknown error");
        append_text(lab->buf, "\n");
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        append_text(lab->buf, "\n✅ done\n");
    }
}

/* Build the LLM Lab UI. */
static GtkWidget *build_lab(LlmLab **out_state)
{
    LlmLab *lab = g_new0(LlmLab, 1);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing   (GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_widget_set_hexpand(grid, TRUE);
    gtk_widget_set_vexpand(grid, TRUE);

    lab->provider_model = gtk_string_list_new(NULL);
    gtk_string_list_append(lab->provider_model, "zai");
    gtk_string_list_append(lab->provider_model, "openai");
    lab->provider = GTK_DROP_DOWN(gtk_drop_down_new(G_LIST_MODEL(lab->provider_model), NULL));
    gtk_drop_down_set_selected(GTK_DROP_DOWN(lab->provider), 0);

    lab->stream_sw = GTK_SWITCH(gtk_switch_new());
    gtk_switch_set_active(lab->stream_sw, TRUE);

    lab->show_alts = GTK_CHECK_BUTTON(
        gtk_check_button_new_with_label("Show token alternatives (when available)")
    );
    gtk_check_button_set_active(lab->show_alts, TRUE);

    GtkWidget *send = gtk_button_new_with_label("Send");
    g_signal_connect(send, "clicked", G_CALLBACK(on_send), lab);

    lab->entry = GTK_ENTRY(gtk_entry_new());
    gtk_editable_set_text(GTK_EDITABLE(lab->entry), "Explain what this project does.");

    /* Output area */
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    lab->out_view = GTK_TEXT_VIEW(gtk_text_view_new());
    lab->buf = gtk_text_view_get_buffer(lab->out_view);
    gtk_text_view_set_wrap_mode(lab->out_view, GTK_WRAP_WORD_CHAR);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), GTK_WIDGET(lab->out_view));

    /* Alternatives area */
    GtkWidget *alts_scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(alts_scroll, TRUE);
    lab->alts_view = GTK_TEXT_VIEW(gtk_text_view_new());
    lab->alts_buf = gtk_text_view_get_buffer(lab->alts_view);
    gtk_text_view_set_wrap_mode(lab->alts_view, GTK_WRAP_WORD_CHAR);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(alts_scroll), GTK_WIDGET(lab->alts_view));

    /* Layout grid:
     *  row 0: [provider] [stream switch] [show_alts]
     *  row 1: [entry.................]   [send]
     *  row 2: [out scroll .........]    [alts scroll]
     */
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(lab->provider), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(lab->stream_sw), 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(lab->show_alts), 2, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(lab->entry),     0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), send,                        2, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), scroll,                      0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), alts_scroll,                 2, 2, 1, 1);

    /* Apply this branch only when its contract condition is satisfied. */
    if (out_state) *out_state = lab;
    return grid;
}

/* ── Public API ─────────────────────────────────────────────────────────── */

GtkWidget *umi_llm_lab_new_with_parent(GtkWindow *parent)
{
    (void)parent; /* reserved: use parent to set transient/modality later */
    LlmLab *st = NULL;
    GtkWidget *w = build_lab(&st);
    /* Optionally store 'st' on the widget if needed by callers:
       g_object_set_data_full(G_OBJECT(w), "llm.lab.state", st, g_free); */
    return w;
}

/* Provide the llm lab present operation used by this module and its client applications. */
void umi_llm_lab_present(GtkWidget *w)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!w) return;
    GtkRoot *root = gtk_widget_get_root(w);
    /* Apply this branch only when its contract condition is satisfied. */
    if (GTK_IS_WINDOW(root)) gtk_window_present(GTK_WINDOW(root));
    /* Use this fallback path when the earlier condition does not apply. */
    else                     gtk_widget_grab_focus(w);
}
