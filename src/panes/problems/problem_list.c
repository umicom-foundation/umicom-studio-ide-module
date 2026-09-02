/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/panes/problems/problem_list.c
 *
 * PURPOSE:
 *   Implement the problem list behavior for
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
 * File: src/panes/problems/problem_list.c
 *
 * PURPOSE:
 *   GTK4 list implementation backing the Problems pane. Accepts UmiDiag
 *   entries and renders them as rows with severity and message; rows carry
 *   jump metadata (file, line, col) for editor navigation.
* DESIGN:
 *   - Strictly use public headers by name; no deep includes.
 *   - Copy incoming UmiDiag data to owned GObjects/strings to avoid aliasing.
 *   - GTK4-only code paths; no deprecated APIs.
 *
 *
 * SECURITY:
 *   - No dynamic format strings; labels created from sanitized inputs.
 *   - All allocations are cleaned; row payload freed on row destroy.
 *
 * Created by: Umicom Foundation | Developer: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/
#include <glib.h>
#include <gtk/gtk.h>
#include "problem_list.h"

typedef struct RowPayload {
    char *file;
    int   line;
    int   col;
} RowPayload;

struct _UmiProblemList {
    GtkWidget            *scroller;
    GtkWidget            *list;
    unsigned              count;
    UmiProblemActivateCb  on_activate;
    gpointer              user;
};

/* signal handler for "row-activated" */
static void on_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    (void)box;
    UmiProblemList *pl = (UmiProblemList*)user_data;
    /* Apply this branch only when its contract condition is satisfied. */
    if (!pl || !pl->on_activate) return;
    RowPayload *p = (RowPayload*)g_object_get_data(G_OBJECT(row), "umi.payload");
    /* Apply this branch only when its contract condition is satisfied. */
    if (!p) return;
    pl->on_activate(pl->user, p->file ? p->file : "", p->line, p->col);
}

/* Provide the row payload free operation used by this module and its client applications. */
static void row_payload_free(gpointer data) {
    RowPayload *p = (RowPayload*)data;
    /* Apply this branch only when its contract condition is satisfied. */
    if (!p) return;
    g_free(p->file);
    g_free(p);
}

/* Provide the mk row operation used by this module and its client applications. */
static GtkWidget *mk_row(const char *severity, const char *file, int line, int col, const char *msg) {
    GtkWidget *row  = gtk_list_box_row_new();
    GtkWidget *box  = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lbl1 = gtk_label_new(severity ? severity : "");
    GtkWidget *lbl2 = gtk_label_new(msg ? msg : "");
    gtk_label_set_xalign(GTK_LABEL(lbl1), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(lbl2), 0.0f);
    gtk_widget_add_css_class(lbl1, "dim-label");

    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);
    gtk_box_append(GTK_BOX(box), lbl1);
    gtk_box_append(GTK_BOX(box), lbl2);

    RowPayload *p = g_new0(RowPayload, 1);
    p->file = g_strdup(file ? file : "");
    p->line = line;
    p->col  = col;
    g_object_set_data_full(G_OBJECT(row), "umi.payload", p, row_payload_free);

    return row;
}

/* public API */
UmiProblemList *umi_problem_list_new(void) {
    return umi_problem_list_new_with_cb(NULL, NULL);
}

/*
 * Provide the problem list new with cb operation used by this module and its client
 * applications.
 */
UmiProblemList *umi_problem_list_new_with_cb(UmiProblemActivateCb cb, gpointer user) {
    UmiProblemList *pl = g_new0(UmiProblemList, 1);
    pl->scroller = gtk_scrolled_window_new();
    pl->list     = gtk_list_box_new();
    pl->on_activate = cb;
    pl->user        = user;

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(pl->scroller), pl->list);
    g_signal_connect(pl->list, "row-activated", G_CALLBACK(on_row_activated), pl);
    return pl;
}

/* Provide the problem list free operation used by this module and its client applications. */
void umi_problem_list_free(UmiProblemList *pl) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (!pl) return;
    /* Apply this branch only when its contract condition is satisfied. */
    if (pl->list) {
        GtkWidget *child = gtk_widget_get_first_child(pl->list);
        /*
         * Continue only while work remains available; the loop body advances the state on each
         * pass.
         */
        while (child) {
            GtkWidget *next = gtk_widget_get_next_sibling(child);
            gtk_list_box_remove(GTK_LIST_BOX(pl->list), child);
            child = next;
        }
        g_object_unref(pl->list);
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (pl->scroller) g_object_unref(pl->scroller);
    g_free(pl);
}

/* Add problem list only after its inputs and available capacity have been checked. */
gboolean umi_problem_list_add(UmiProblemList *pl, const UmiDiag *diag) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (!pl || !pl->list || !diag) return FALSE;
    const char *file = diag->file ? diag->file : "";
    const char *msg  = diag->message ? diag->message : "";
    const char *sev  =
        (diag->severity == UMI_DIAG_ERROR)   ? "error"   :
        (diag->severity == UMI_DIAG_WARNING) ? "warning" : "info";
    GtkWidget *row = mk_row(sev, file, (int)diag->line, (int)diag->column, msg);
    gtk_list_box_append(GTK_LIST_BOX(pl->list), row);
    pl->count++;
    return TRUE;
}

/* Release or reset state held by problem list so the same storage can be reused safely. */
unsigned umi_problem_list_clear(UmiProblemList *pl) {
    /* Apply this branch only when its contract condition is satisfied. */
    if (!pl || !pl->list) return 0u;
    unsigned removed = 0;
    GtkWidget *child = gtk_widget_get_first_child(pl->list);
    /*
     * Continue only while work remains available; the loop body advances the state on each
     * pass.
     */
    while (child) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(GTK_LIST_BOX(pl->list), child);
        child = next;
        removed++;
    }
    pl->count = 0;
    return removed;
}

/* Return the number of records represented by problem list without changing their state. */
unsigned umi_problem_list_count(UmiProblemList *pl) {
    return pl ? pl->count : 0u;
}

/*
 * Provide the problem list model operation used by this module and its client
 * applications.
 */
const void *umi_problem_list_model(UmiProblemList *pl) {
    return pl ? (const void*)pl->list : NULL;
}

/*
 * Provide the problem list widget operation used by this module and its client
 * applications.
 */
GtkWidget *umi_problem_list_widget(UmiProblemList *pl) {
    return pl ? pl->scroller : NULL;
}