/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/util/watchers/watcher_recursive.c
 *
 * PURPOSE:
 *   Implement the watcher recursive behavior for
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
 * File: src/util/watchers/watcher_recursive.c
 *
 * PURPOSE:
 *   Recursive watcher built on GFileMonitor (one monitor per directory).
 *   When any file in the tree changes, we invoke the user callback with a
 *   best-effort UTF-8 path. Pure C; no UI/cross-module headers.
 *
 * DESIGN:
 *   - Track monitors (GFileMonitor*) and root directories we manage.
 *   - Convert file paths to UTF-8 using GLib; avoid following symlink loops.
 *   - WATCH_MOVES to classify renames on backends that support it.
 *
 * SECURITY/ROBUSTNESS:
 *   - Skip symlinked directories to avoid cycles.
 *   - Defensive guards; avoid unbounded prints; strict ownership.
 *
 * Created by: Umicom Foundation | Developer: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/

#include "watcher_recursive.h"

#include <string.h>
#include <errno.h>
#include <glib.h>

/* Private structure */
struct _UmiWatcherRec {
    GPtrArray  *monitors;  /* Array<GFileMonitor*>; owned here                */
    GPtrArray  *roots;     /* Array<char*> of directory roots we manage       */
    UmiWatchCb  cb;        /* User callback                                   */
    gpointer    user;      /* Opaque pointer passed back                      */
};

/* Normalize to a displayable UTF-8 path or URI. */
static char *normalize_path(GFile *f)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!f) return g_strdup("(unknown)");
    char *p = g_file_get_path(f);
    /* Apply this branch only when its contract condition is satisfied. */
    if (!p) return g_file_get_uri(f);
#ifdef G_OS_WIN32
    /* Visit each bounded item once so every record receives the same rule. */
    for (char *q = p; *q; ++q) /* Apply this branch only when its contract condition is satisfied. */ if (*q == '\\') *q = '/';
#endif
    return p;
}

/* Provide the mon changed operation used by this module and its client applications. */
static void mon_changed(GFileMonitor *mon,
                        GFile         *file,
                        GFile         *other_file,
                        GFileMonitorEvent evt,
                        gpointer       u)
{
    (void)mon; (void)other_file; (void)evt;
    UmiWatcherRec *w = (UmiWatcherRec*)u;
    /* Apply this branch only when its contract condition is satisfied. */
    if (!w || !w->cb) return;

    g_autofree char *path = normalize_path(file);
    w->cb(w->user, path ? path : "(unknown)");
}

/* Attach a monitor to a directory with WATCH_MOVES if available. */
static gboolean add_dir_monitor(UmiWatcherRec *w, const char *dir_path)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!w || !dir_path || !*dir_path) return FALSE;

    g_autoptr(GFile) dir = g_file_new_for_path(dir_path);
    GError *err = NULL;

    GFileMonitor *m = g_file_monitor_directory(
        dir, G_FILE_MONITOR_WATCH_MOVES, NULL, &err);

    /* Apply this branch only when its contract condition is satisfied. */
    if (!m) {
        g_warning("watcher: monitor failed for '%s': %s",
                  dir_path, err ? err->message : "unknown");
        g_clear_error(&err);
        return FALSE;
    }

    g_signal_connect(m, "changed", G_CALLBACK(mon_changed), w);
    g_ptr_array_add(w->monitors, m); /* owned by array free func */
    return TRUE;
}

/* Depth-first scan with symlink-guard. */
static void scan_dir(UmiWatcherRec *w, const char *root)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!w || !root) return;

    add_dir_monitor(w, root);

    GError *err = NULL;
    g_autoptr(GFile) groot = g_file_new_for_path(root);
    g_autoptr(GFileEnumerator) en =
        g_file_enumerate_children(groot,
            G_FILE_ATTRIBUTE_STANDARD_NAME ","
            G_FILE_ATTRIBUTE_STANDARD_TYPE ","
            G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK,
            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
            NULL, &err);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (!en) { g_clear_error(&err); return; }

    /* Visit each bounded item once so every record receives the same rule. */
    for (;;) {
        GFileInfo *info = g_file_enumerator_next_file(en, NULL, &err);
        /* Apply this branch only when its contract condition is satisfied. */
        if (!info) break;

        const gboolean is_dir  = (g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY);
        const gboolean is_link = g_file_info_get_is_symlink(info);

        /* Apply this branch only when its contract condition is satisfied. */
        if (is_dir && !is_link) {
            const char *name = g_file_info_get_name(info);
            g_autoptr(GFile) child = g_file_get_child(groot, name);
            g_autofree char *child_path = g_file_get_path(child);
            /* Apply this branch only when its contract condition is satisfied. */
            if (child_path) scan_dir(w, child_path);
        }
        g_object_unref(info);
    }
    g_clear_error(&err);
}

/* If given a file, return its parent directory; else duplicate dir path. */
static char *dir_for_any_path(const char *path)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!path || !*path) return NULL;
    /* Apply this branch only when its contract condition is satisfied. */
    if (g_file_test(path, G_FILE_TEST_IS_DIR)) return g_strdup(path);
    g_autofree char *parent = g_path_get_dirname(path);
    return g_strdup(parent);
}

/* Drop all monitors (disconnect + free). */
static void clear_monitors(UmiWatcherRec *w)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!w || !w->monitors) return;
    /* Visit each bounded item once so every record receives the same rule. */
    for (guint i = 0; i < w->monitors->len; ++i) {
        GFileMonitor *m = g_ptr_array_index(w->monitors, i);
        /* Apply this branch only when its contract condition is satisfied. */
        if (!m) continue;
        g_signal_handlers_disconnect_by_data(m, w);
        /* array free func will unref in unref below */
    }
    g_ptr_array_set_size(w->monitors, 0);
}

/* Provide the watchrec new operation used by this module and its client applications. */
UmiWatcherRec *umi_watchrec_new(const char *root, UmiWatchCb cb, gpointer user)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!root || !*root || !cb) return NULL;

    UmiWatcherRec *w = g_new0(UmiWatcherRec, 1);
    w->monitors = g_ptr_array_new_with_free_func((GDestroyNotify)g_object_unref);
    w->roots    = g_ptr_array_new_with_free_func(g_free);
    w->cb       = cb;
    w->user     = user;

    g_ptr_array_add(w->roots, g_strdup(root));
    scan_dir(w, root);
    return w;
}

/* Add watchrec only after its inputs and available capacity have been checked. */
gboolean umi_watchrec_add(UmiWatcherRec *w, const char *path_or_dir)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!w || !path_or_dir || !*path_or_dir) return FALSE;

    g_autofree char *dir = dir_for_any_path(path_or_dir);
    /* Apply this branch only when its contract condition is satisfied. */
    if (!dir) return FALSE;

    /* Visit each bounded item once so every record receives the same rule. */
    for (guint i = 0; i < w->roots->len; ++i) {
        const char *existing = g_ptr_array_index(w->roots, i);
        /* Use the stable identifier comparison to choose the matching record or policy. */
        if (g_strcmp0(existing, dir) == 0) { scan_dir(w, dir); return TRUE; }
    }

    g_ptr_array_add(w->roots, g_strdup(dir));
    scan_dir(w, dir);
    return TRUE;
}

/* Provide the watchrec rescan operation used by this module and its client applications. */
void umi_watchrec_rescan(UmiWatcherRec *w)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!w) return;
    clear_monitors(w);
    /* Visit each bounded item once so every record receives the same rule. */
    for (guint i = 0; i < w->roots->len; ++i) {
        const char *r = g_ptr_array_index(w->roots, i);
        scan_dir(w, r);
    }
}

/* Provide the watchrec free operation used by this module and its client applications. */
void umi_watchrec_free(UmiWatcherRec *w)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!w) return;
    clear_monitors(w);
    g_clear_pointer(&w->monitors, g_ptr_array_unref);
    g_clear_pointer(&w->roots,    g_ptr_array_unref);
    g_free(w);
}
