/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/util/fs/file_index.c
 *
 * PURPOSE:
 *   Implement the file index behavior for
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
 * File: src/util/fs/file_index.c
 *
 * PURPOSE:
 *   Maintain a simple in-memory index of files under a root directory.
 *   - Build the index from disk (recursive scan via fs_walk)
 *   - Refresh it (clear + rescan)
 *   - Free all resources
 *
 * NOTES:
 *   - Matches the public API in src/util/fs/include/file_index.h exactly:
 *       UmiFileIndex *umi_index_build(const char *root);
 *       void          umi_index_refresh(UmiFileIndex *idx);
 *       void          umi_index_free(UmiFileIndex *idx);
 *   - Stores g_strdup'd, canonicalized (normalized) file paths in a GPtrArray.
 *   - No GTK dependencies; pure GLib.
 *
 * THREADING:
 *   - Synchronous; call off the GTK main loop if the tree is large.
 *
 * Created by: Umicom Foundation | Developer: Sammy Hegab | Date: 2025-10-12 | MIT
 *---------------------------------------------------------------------------*/
#include "include/file_index.h"
#include "include/fs_walk.h"
#include <string.h>

/*---------------------------------------------------------------------------
 * Private structure for the index. Kept internal to avoid ABI churn.
 *-------------------------------------------------------------------------*/
struct _UmiFileIndex {
    char      *root;     /* Absolute, canonical root directory path      */
    GPtrArray *files;    /* Owned array of g_strdup'd file paths         */
};

/*---------------------------------------------------------------------------
 * Helper: free all file strings and reset the array to empty.
 *-------------------------------------------------------------------------*/
static void
clear_files(UmiFileIndex *idx)
{
    /* The array was created with g_free as its element destructor. Reducing
     * its size therefore releases every removed path exactly once. Manually
     * freeing each path first would leave the array holding dangling pointers
     * and would make this call free the same memory a second time. */
    if (idx == NULL || idx->files == NULL) {
        return;
    }
    g_ptr_array_set_size(idx->files, 0);
}

/*---------------------------------------------------------------------------
 * Walker callback: collect only REGULAR FILES (not directories).
 *-------------------------------------------------------------------------*/
static void
on_visit(const char *path, gboolean is_dir, gpointer user)
{
    UmiFileIndex *idx = (UmiFileIndex *)user;
    /* Apply this branch only when its contract condition is satisfied. */
    if (!idx || !idx->files) return;

    /* Apply this branch only when its contract condition is satisfied. */
    if (is_dir) {
        return;                                         /* ignore directories   */
    }
    /* Own a duplicate of the canonicalized path. */
    g_ptr_array_add(idx->files, g_strdup(path));
}

/*---------------------------------------------------------------------------
 * Stable sort: correctly-typed comparator (no casting warnings).
 *-------------------------------------------------------------------------*/
static gint
cmp_paths(gconstpointer a, gconstpointer b, gpointer user_data)
{
    const char *const *left_path = (const char *const *)a;
    const char *const *right_path = (const char *const *)b;

    (void)user_data;

    /* GPtrArray passes the addresses of its pointer slots to the comparator.
     * Dereferencing each slot produces the actual path string that should be
     * compared. Treating the slot address itself as text reads unrelated
     * memory and can cause unstable ordering or an access violation. */
    return g_strcmp0(*left_path, *right_path);
}

/*---------------------------------------------------------------------------
 * Public: build a new index from 'root'.
 *-------------------------------------------------------------------------*/
UmiFileIndex *
umi_index_build(const char *root)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!root || !*root) return NULL;

    UmiFileIndex *idx = g_new0(UmiFileIndex, 1);
    idx->root  = g_canonicalize_filename(root, NULL);
    idx->files = g_ptr_array_new_with_free_func(g_free);

    /* Walk and collect; include_hidden = FALSE by default. */
    if (!umi_fs_walk(idx->root, FALSE, on_visit, idx)) {
        /* If walking fails, return a valid (empty) index so callers can still
         * inspect 'root' and later attempt refresh().
         */
    }

    /* Sort for deterministic iteration order. */
    g_ptr_array_sort_with_data(idx->files, cmp_paths, NULL);
    return idx;
}

/*---------------------------------------------------------------------------
 * Public: refresh the existing index by clearing and re-walking.
 *-------------------------------------------------------------------------*/
void
umi_index_refresh(UmiFileIndex *idx)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!idx) return;

    clear_files(idx);
    (void)umi_fs_walk(idx->root, FALSE, on_visit, idx);
    g_ptr_array_sort_with_data(idx->files, cmp_paths, NULL);
}

/*---------------------------------------------------------------------------
 * Public: free the index and all owned data.
 *-------------------------------------------------------------------------*/
void
umi_index_free(UmiFileIndex *idx)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!idx) return;

    clear_files(idx);
    /* Apply this branch only when its contract condition is satisfied. */
    if (idx->files) g_ptr_array_free(idx->files, TRUE);
    g_clear_pointer(&idx->root, g_free);
    g_free(idx);
}

/*---------------------------------------------------------------------------
 * Public helper: expose a read-only view (non-owning) of the paths, for panels
 * that want to render lists without copying.
 *-------------------------------------------------------------------------*/
const char * const *
umi_index_files_real(const UmiFileIndex *idx, guint *out_len)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!idx || !idx->files) {
        /* Apply this branch only when its contract condition is satisfied. */
        if (out_len) *out_len = 0;
        return NULL;
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (out_len) *out_len = idx->files->len;
    return (const char * const *)idx->files->pdata;
}
