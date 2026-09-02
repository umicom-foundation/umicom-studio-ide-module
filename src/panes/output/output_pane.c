/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/panes/output/output_pane.c
 *
 * PURPOSE:
 *   Implement the output pane behavior for
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
 * File: src/output_pane.c
 * PURPOSE: Implements a scrollable text console for build/run output
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/
#include "output_pane.h"
#include <string.h>

struct _UmiOutputPane {
  GtkWidget *root;
  GtkWidget *scroller;
  GtkWidget *view;
  GtkTextBuffer *buf;
};

/* Provide the append text operation used by this module and its client applications. */
static void append_text(GtkTextBuffer *b, const char *s){
  /* Apply this branch only when its contract condition is satisfied. */
  if(!b || !s) return;
  GtkTextIter end; gtk_text_buffer_get_end_iter(b, &end);
  gtk_text_buffer_insert(b, &end, s, -1);
}

/* Provide the output pane new operation used by this module and its client applications. */
UmiOutputPane* umi_output_pane_new(void){
  UmiOutputPane *p = g_new0(UmiOutputPane, 1);
  p->scroller = gtk_scrolled_window_new();
  p->buf = gtk_text_buffer_new(NULL);
  p->view = gtk_text_view_new_with_buffer(p->buf);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(p->view), FALSE);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(p->view), TRUE);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(p->scroller), p->view);
  p->root = p->scroller;
  return p;
}

/*
 * Provide the output pane widget operation used by this module and its client
 * applications.
 */
GtkWidget* umi_output_pane_widget(UmiOutputPane *p){ return p ? p->root : NULL; }
/* Release or reset state held by output pane so the same storage can be reused safely. */
void umi_output_pane_clear(UmiOutputPane *p){ /* Apply this branch only when its contract condition is satisfied. */ if(!p) return; gtk_text_buffer_set_text(p->buf, "", -1); }
/* Add output pane only after its inputs and available capacity have been checked. */
void umi_output_pane_append(UmiOutputPane *p, const char *text){ /* Apply this branch only when its contract condition is satisfied. */ if(!p) return; append_text(p->buf, text); }
/*
 * Provide the output pane append line operation used by this module and its client
 * applications.
 */
void umi_output_pane_append_line(UmiOutputPane *p, const char *text){ /* Apply this branch only when its contract condition is satisfied. */ if(!p) return; append_text(p->buf, text); append_text(p->buf, "\n"); }
/*
 * Provide the output pane append line err operation used by this module and its client
 * applications.
 */
void umi_output_pane_append_line_err(UmiOutputPane *p, const char *text){ /* Apply this branch only when its contract condition is satisfied. */ if(!p) return; append_text(p->buf, "[err] "); append_text(p->buf, text); append_text(p->buf, "\n"); }
