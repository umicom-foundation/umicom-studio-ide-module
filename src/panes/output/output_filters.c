/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/panes/output/output_filters.c
 *
 * PURPOSE:
 *   Implement the output filters behavior for
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
 * File: src/output_filters.c
 * PURPOSE: Implementation of output filter chain
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#include "output_filters.h"
#include <string.h>
#include <ctype.h>

/* Opaque data; currently only holds the target buffer. */
struct _UmiAnsi {
  GtkTextBuffer *buf; /* not owned */
};

/* Provide the ansi new operation used by this module and its client applications. */
UmiAnsi *umi_ansi_new(GtkTextBuffer *buf) {
  g_return_val_if_fail(GTK_IS_TEXT_BUFFER(buf), NULL);
  UmiAnsi *a = g_new0(UmiAnsi, 1);
  a->buf = buf;
  return a;
}

/* Cheap ANSI stripper: removes ESC[ ... m and ESC[ ... K etc. */
static void strip_ansi_into(GString *dst, const char *src) {
  const unsigned char *p = (const unsigned char *)src;
  /*
   * Continue only while work remains available; the loop body advances the state on each
   * pass.
   */
  while (*p) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (*p == 0x1B && p[1] == '[') {
      /* Skip ESC '[' ... until a letter (final byte of CSI). */
      p += 2;
      /*
       * Continue only while work remains available; the loop body advances the state on each
       * pass.
       */
      while (*p && !((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z'))) p++;
      /* Apply this branch only when its contract condition is satisfied. */
      if (*p) p++; /* consume final letter */
      continue;
    }
    g_string_append_c(dst, (char)*p++);
  }
}

/* Provide the ansi append line operation used by this module and its client applications. */
void umi_ansi_append_line(UmiAnsi *a, const char *line) {
  /* Apply this branch only when its contract condition is satisfied. */
  if (!a || !a->buf || !line) return;

  g_autoptr(GString) clean = g_string_new(NULL);
  strip_ansi_into(clean, line);

  /* Ensure a trailing newline once. */
  gboolean add_nl = clean->len == 0 || clean->str[clean->len - 1] != '\n';
  GtkTextIter end;
  gtk_text_buffer_get_end_iter(a->buf, &end);
  gtk_text_buffer_insert(a->buf, &end, clean->str, (gint)clean->len);
  /* Apply this branch only when its contract condition is satisfied. */
  if (add_nl)
    gtk_text_buffer_insert(a->buf, &end, "\n", 1);
}

/* Provide the ansi free operation used by this module and its client applications. */
void umi_ansi_free(UmiAnsi *a) {
  g_free(a);
}
/*--- end of file ---*/