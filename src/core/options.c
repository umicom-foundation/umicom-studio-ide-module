/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/options.c
 *
 * PURPOSE:
 *   Implement the options behavior for
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
 * File: src/options.c
 * PURPOSE: Implementation of runtime options parsing
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#include "options.h"

/* Read options into validated module state and return a status when input cannot be used. */
UmiOptions *umi_options_parse(int *argc, char ***argv, GError **err){
  UmiOptions *o = g_new0(UmiOptions,1);
  o->verbose = FALSE; o->log_level = 1; o->headless = FALSE; o->workspace = NULL;

  GOptionEntry entries[] = {
    { "verbose",  'v', 0, G_OPTION_ARG_NONE,   &o->verbose,  "Enable verbose logs", NULL },
    { "log",      'l', 0, G_OPTION_ARG_INT,    &o->log_level,"Log level (0=DEBUG..3=ERROR)", "N" },
    { "workspace",'w', 0, G_OPTION_ARG_STRING, &o->workspace,"Workspace root", "DIR" },
    { "headless", 0,   0, G_OPTION_ARG_NONE,   &o->headless, "Run without UI", NULL },
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
  };
  GOptionContext *ctx = g_option_context_new("- Umicom Studio IDE options");
  g_option_context_add_main_entries(ctx, entries, NULL);
  gboolean ok = g_option_context_parse(ctx, argc, argv, err);
  g_option_context_free(ctx);
  /* Preserve the original failure result so the caller can respond to the correct cause. */
  if(!ok){ umi_options_free(o); return NULL; }
  return o;
}

/* Provide the options free operation used by this module and its client applications. */
void umi_options_free(UmiOptions *o){
  /* Apply this branch only when its contract condition is satisfied. */
  if(!o) return;
  g_free(o->workspace);
  g_free(o);
}
