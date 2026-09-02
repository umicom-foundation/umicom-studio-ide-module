/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/util/watchers/watcher_integration.c
 *
 * PURPOSE:
 *   Implement the watcher integration behavior for
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
 * File: src/util/watchers/watcher_integration.c
 *
 * PURPOSE:
 *   Integration layer: owns a recursive watcher and routes its callback to
 *   refresh UI components (FileTree), optionally touching Workspace state.
 *
 * DESIGN:
 *   - Very thin by design; keep work minimal in callback.
 *   - Future: debounce/throttle refresh via g_timeout_add if needed.
 *
 * Created by: Umicom Foundation | Developer: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/

#include "watcher_integration.h"
#include <gio/gio.h>
#include <glib.h>

/* Forward declare UI refresh to avoid heavy includes. */
void umi_file_tree_refresh(struct _FileTree *tree);

struct _UmiWatcherIntegration {
    FileTree       *tree;   /* borrowed */
    WorkspaceState *ws;     /* borrowed (unused for now) */
    UmiWatcherRec  *rec;    /* owned */
};

/* Provide the on evt operation used by this module and its client applications. */
static void on_evt(gpointer u, const char *path)
{
    (void)path; /* refresh whole tree; future: selective refresh */
    UmiWatcherIntegration *g = (UmiWatcherIntegration*)u;
    /* Apply this branch only when its contract condition is satisfied. */
    if (!g || !g->tree) return;

    /* If you must marshal to GTK main thread explicitly, use g_idle_add(). */
    umi_file_tree_refresh(g->tree);
}

/* Provide the watch integ new operation used by this module and its client applications. */
UmiWatcherIntegration *umi_watch_integ_new(FileTree *tree, WorkspaceState *ws)
{
    UmiWatcherIntegration *wi = g_new0(UmiWatcherIntegration, 1);
    wi->tree = tree;
    wi->ws   = ws;
    wi->rec  = NULL;
    return wi;
}

/* Add watch integ only after its inputs and available capacity have been checked. */
gboolean umi_watch_integ_add(UmiWatcherIntegration *wi, const UmiPathWatch *req)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!wi || !req || !req->path || !*req->path) return FALSE;

    /* Apply this branch only when its contract condition is satisfied. */
    if (!wi->rec) {
        wi->rec = umi_watchrec_new(req->path, on_evt, wi);
        /* Apply this branch only when its contract condition is satisfied. */
        if (!wi->rec) return FALSE;
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        /* Apply this branch only when its contract condition is satisfied. */
        if (!umi_watchrec_add(wi->rec, req->path)) return FALSE;
    }
    (void)req->recursive; /* recursive by construction */
    return TRUE;
}

/* Provide the watch integ free operation used by this module and its client applications. */
void umi_watch_integ_free(UmiWatcherIntegration *wi)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!wi) return;
    /* Apply this branch only when its contract condition is satisfied. */
    if (wi->rec) { umi_watchrec_free(wi->rec); wi->rec = NULL; }
    g_free(wi);
}
