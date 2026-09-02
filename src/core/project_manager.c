/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/project_manager.c
 *
 * PURPOSE:
 *   Implement the project manager behavior for
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
 * File: src/core/project_manager.c
 * PURPOSE: Coordinate the workspace root, recursive file index, recent-work
 *          history and optional status line when a project folder is opened.
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-12 | MIT
 *---------------------------------------------------------------------------
 * RELATIONSHIPS:
 * - UmiWorkspace owns the selected root and tells the file-tree view to reload.
 * - UmiFileIndex owns a searchable snapshot of files below that root.
 * - UmiRecent records successfully opened roots for the welcome screen.
 * - UmiStatusBar is optional and mirrors progress to the visible status line.
 *---------------------------------------------------------------------------*/

#include <glib.h>                 /* gboolean, gchar, GError, g_set_error, etc.  */
#include <stdarg.h>               /* va_list used to format detailed errors.      */

#include "project_manager.h"      /* Must match this header’s struct + prototypes */

/*-----------------------------------------------------------------------------
 * Internal helpers
 *---------------------------------------------------------------------------*/

/* Small, static error domain so callers can distinguish our errors if desired. */
static GQuark
umi_project_error_quark(void)
{
    /* Using a fixed string is fine; this becomes interned as a GQuark.        */
    return g_quark_from_static_string("uside-project-error");
}

/* Set a formatted error only when the caller supplied an empty error slot. */
static void
umi_project_set_error(GError **err, int code, const char *fmt, ...)
{
    va_list ap;
    gchar *msg;

    /* GLib expects an error output slot to be empty before a function writes
     * into it. Keeping an existing error untouched preserves the first failure
     * and avoids a diagnostic from g_set_error(). */
    if (err == NULL || *err != NULL) {
        return;
    }
    va_start(ap, fmt);
    msg = g_strdup_vprintf(fmt, ap);
    va_end(ap);

    g_set_error(err, umi_project_error_quark(), code, "%s", msg ? msg : "error");
    g_free(msg);
}

/* Validate that a path exists and is a directory.                             */
static gboolean
umi_project_validate_dir(const char *folder, GError **err)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (G_UNLIKELY(folder == NULL || *folder == '\0')) {
        umi_project_set_error(err, 1, "No folder path provided");
        return FALSE;
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (!g_file_test(folder, G_FILE_TEST_IS_DIR)) {
        umi_project_set_error(err, 2, "Folder is not a directory or not accessible: %s", folder);
        return FALSE;
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------
 * Constructor
 *---------------------------------------------------------------------------*/
UmiProjectManager *
umi_project_manager_new(
    UmiWorkspace *ws,
    UmiRecent *recent,
    UmiStatusBar *status)
{
    /* Allocate the manager; fields not explicitly set are zero-initialised.    */
    UmiProjectManager *pm = g_new0(UmiProjectManager, 1);

    /* The workspace, recent-work registry and status wrapper are borrowed.
     * Their owners may share them with the welcome page and other panels. */
    pm->ws     = ws;       /* may be NULL if not initialised yet               */
    pm->recent = recent;   /* may be NULL                                      */
    pm->status = status;   /* may be NULL for command-line or test sessions     */

    /* Index is lazily (re)built; keep NULL until refresh/open decides.         */
    pm->index  = NULL;

    /* Informative log without requiring a logging subsystem.                   */
    g_message("UmiProjectManager: created (ws=%p, recent=%p, status=%p)", (void*)ws, (void*)recent, (void*)status);
    return pm;
}

/*-----------------------------------------------------------------------------
 * Open a folder as the workspace root
 *---------------------------------------------------------------------------*/
gboolean
umi_project_open(UmiProjectManager *pm, const char *folder, GError **err)
{
    gchar *canonical_folder;
    UmiFileIndex *new_index;
    UmiFileIndex *old_index;
    guint file_count = 0U;
    gchar *status_message;

    /* A project manager cannot do useful work without its workspace model. */
    g_return_val_if_fail(pm != NULL, FALSE);
    if (pm->ws == NULL) {
        umi_project_set_error(err, 3, "The project manager has no workspace model");
        return FALSE;
    }

    /* Validate directory path and existence; set GError on failure.            */
    if (!umi_project_validate_dir(folder, err)) {
        return FALSE;
    }

    /* Canonicalising once gives the workspace, index and recent-work registry
     * the same stable path even when the user enters relative path segments. */
    canonical_folder = g_canonicalize_filename(folder, NULL);
    if (canonical_folder == NULL) {
        umi_project_set_error(err, 4, "The project folder path could not be normalised");
        return FALSE;
    }

    /* Build the replacement before changing visible workspace state. If the
     * allocation fails, the currently open workspace and index remain usable. */
    new_index = umi_index_build(canonical_folder);
    if (new_index == NULL) {
        umi_project_set_error(err, 5, "The project file index could not be created: %s",
                              canonical_folder);
        g_free(canonical_folder);
        return FALSE;
    }

    old_index = pm->index;
    pm->index = new_index;
    umi_workspace_set_root(pm->ws, canonical_folder);
    umi_index_free(old_index);

    /* Reading the count does not transfer ownership of any indexed path. */
    (void)umi_index_files(pm->index, &file_count);

    /* Record a successful workspace open only after validation. Framework
     * owns ordering and atomic persistence through Studio's compatibility
     * adapter, so other launch surfaces can present the same recent work. */
    if (pm->recent != NULL) {
        umi_recent_add(pm->recent, canonical_folder);
        if (!umi_recent_save(pm->recent)) {
            g_warning("UmiProjectManager: recent work could not be saved");
        }
    }

    /* The status line is optional, so the same project service remains useful
     * in headless tests and command-line sessions. */
    status_message = g_strdup_printf("Workspace ready — %u file%s indexed",
                                     file_count,
                                     file_count == 1U ? "" : "s");
    if (pm->status != NULL) {
        umi_status_push(pm->status, status_message);
    }
    g_message("UmiProjectManager: opened '%s' (%u indexed files)",
              canonical_folder, file_count);
    g_free(status_message);
    g_free(canonical_folder);

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * Refresh the file index for the current workspace root.
 *---------------------------------------------------------------------------*/
void
umi_project_refresh_index(UmiProjectManager *pm)
{
    const char *root;
    guint file_count = 0U;
    gchar *status_message;

    g_return_if_fail(pm != NULL);

    /* Refresh needs an open root. A clear status message is more useful than
     * silently leaving stale results when no project has been selected. */
    root = pm->ws != NULL ? umi_workspace_root(pm->ws) : NULL;
    if (root == NULL || root[0] == '\0') {
        if (pm->status != NULL) {
            umi_status_push(pm->status, "Open a workspace before refreshing files");
        }
        g_warning("UmiProjectManager: refresh requested without an open workspace");
        return;
    }

    /* Reuse the existing index so panels holding its borrowed view can request
     * fresh contents. A restored workspace may not have an index yet, in which
     * case the first refresh creates it. */
    if (pm->index != NULL) {
        umi_index_refresh(pm->index);
    } else {
        pm->index = umi_index_build(root);
    }
    if (pm->index == NULL) {
        if (pm->status != NULL) {
            umi_status_push(pm->status, "The workspace file index could not be refreshed");
        }
        g_warning("UmiProjectManager: index creation failed for '%s'", root);
        return;
    }

    (void)umi_index_files(pm->index, &file_count);
    status_message = g_strdup_printf("File index refreshed — %u file%s",
                                     file_count,
                                     file_count == 1U ? "" : "s");
    if (pm->status != NULL) {
        umi_status_push(pm->status, status_message);
    }
    g_message("UmiProjectManager: refreshed '%s' (%u indexed files)",
              root, file_count);
    g_free(status_message);
}

/*-----------------------------------------------------------------------------
 * Destructor
 *---------------------------------------------------------------------------*/
void
umi_project_manager_free(UmiProjectManager *pm)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (pm == NULL) {
        return; /* tolerate NULL for convenience                                */
    }

    /* The manager owns only its file index. Workspace, recent work and status
     * remain alive because their application-level owners may still use them. */
    umi_index_free(pm->index);
    pm->index = NULL;

    g_message("UmiProjectManager: destroyed");
    g_free(pm);
}
/*--- end of file ---*/
