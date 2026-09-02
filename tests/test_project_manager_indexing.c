/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_project_manager_indexing.c
 *
 * PURPOSE:
 *   Verify that opening a workspace builds a deterministic file index, that a
 *   later refresh discovers new files, and that cleanup releases every indexed
 *   path exactly once. Including the Framework status header alongside the
 *   Studio project contract also guards against a return-code type collision.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include <assert.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>

#include "project_manager.h"
#include "umicom/base/status.h"

/* Write a small fixture file and stop immediately if the test environment
 * cannot create it. The contents are unimportant because the index records
 * paths rather than parsing source text. */
static void write_fixture(const char *path)
{
    GError *error = NULL;
    gboolean written;

    assert(path != NULL);
    written = g_file_set_contents(path, "fixture\n", -1, &error);
    assert(written);
    assert(error == NULL);
}

/* Remove only the temporary files and directories created by this test. Each
 * path is explicit so the cleanup can never reach the source workspace. */
static void remove_fixture_tree(const char *temporary_root,
                                const char *project_root,
                                const char *first_file,
                                const char *second_file,
                                const char *third_file)
{
    gchar *workspace_file = g_build_filename(
        temporary_root, "config", "workspace.json", NULL);
    gchar *config_directory = g_build_filename(
        temporary_root, "config", NULL);

    (void)g_remove(first_file);
    (void)g_remove(second_file);
    (void)g_remove(third_file);
    (void)g_remove(workspace_file);
    (void)g_rmdir(project_root);
    (void)g_rmdir(config_directory);
    (void)g_rmdir(temporary_root);

    g_free(config_directory);
    g_free(workspace_file);
}

/* Exercise the real project-manager path without constructing any graphical
 * widgets. The temporary working directory keeps workspace persistence away
 * from both the source checkout and a developer's personal configuration. */
int main(void)
{
    GError *error = NULL;
    gchar *temporary_root = g_dir_make_tmp(
        "umicom-studio-project-manager-XXXXXX", &error);
    gchar *project_root;
    gchar *first_file;
    gchar *second_file;
    gchar *third_file;
    gchar *original_directory;
    gchar *canonical_project;
    UmiWorkspace *workspace;
    UmiProjectManager *manager;
    const char *const *files;
    guint file_count = 0U;
    UmiStatus framework_status = UMI_STATUS_OK;
    gboolean opened;
    int directory_result;

    assert(error == NULL);
    assert(temporary_root != NULL);
    project_root = g_build_filename(temporary_root, "project", NULL);
    first_file = g_build_filename(project_root, "alpha.c", NULL);
    second_file = g_build_filename(project_root, "bravo.c", NULL);
    third_file = g_build_filename(project_root, "charlie.c", NULL);
    directory_result = g_mkdir(project_root, 0700);
    assert(directory_result == 0);
    write_fixture(second_file);
    write_fixture(first_file);

    original_directory = g_get_current_dir();
    directory_result = g_chdir(temporary_root);
    assert(directory_result == 0);

    workspace = umi_workspace_new(NULL);
    manager = umi_project_manager_new(workspace, NULL, NULL);
    assert(workspace != NULL);
    assert(manager != NULL);
    opened = umi_project_open(manager, project_root, &error);
    assert(opened);
    assert(error == NULL);

    canonical_project = g_canonicalize_filename(project_root, NULL);
    assert(g_strcmp0(umi_workspace_root(workspace), canonical_project) == 0);
    files = umi_index_files(manager->index, &file_count);
    assert(files != NULL);
    assert(file_count == 2U);
    assert(g_strcmp0(files[0], files[1]) < 0);

    write_fixture(third_file);
    umi_project_refresh_index(manager);
    files = umi_index_files(manager->index, &file_count);
    assert(files != NULL);
    assert(file_count == 3U);
    assert(g_strcmp0(files[0], files[1]) < 0);
    assert(g_strcmp0(files[1], files[2]) < 0);
    assert(framework_status == UMI_STATUS_OK);

    umi_project_manager_free(manager);
    umi_workspace_free(workspace);
    directory_result = g_chdir(original_directory);
    assert(directory_result == 0);

    remove_fixture_tree(temporary_root, project_root,
                        first_file, second_file, third_file);
    g_free(canonical_project);
    g_free(original_directory);
    g_free(third_file);
    g_free(second_file);
    g_free(first_file);
    g_free(project_root);
    g_free(temporary_root);
    return 0;
}
