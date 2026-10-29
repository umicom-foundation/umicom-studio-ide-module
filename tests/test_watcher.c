/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_watcher.c
 *
 * PURPOSE:
 *   Verify that Studio watcher scans update operational statistics and the
 *   shared workspace file index.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* These assertions perform setup as well as checks, including in release builds. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "studio_test_state.h"
#include "umicom/studio/watcher.h"
#include "umicom/studio/workspace.h"

#include <assert.h>
#include <stdio.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    /* Isolate default preferences and data before starting shared services. */
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;

    UmiStudioServices *services = NULL;
    UmiWatcherStats watcher_stats;
    UmiFileIndexStats index_stats;
    char temporary[UMI_PATH_CAPACITY];
    char root[UMI_PATH_CAPACITY];
    char file[UMI_PATH_CAPACITY];

    /* Fixed fixture names belong beneath this invocation, not the shared temp root.
     * Retain the old base selection for review of the ownership correction. */
#if 0
    assert(umi_fs_temp_directory(temporary, sizeof(temporary)) == UMI_STATUS_OK);
#endif
    assert(umi_path_copy(temporary, sizeof(temporary), test_state.directory) == UMI_STATUS_OK);
    assert(umi_path_join(temporary,
                         "umicom-studio-watcher-test",
                         root,
                         sizeof(root)) == UMI_STATUS_OK);
    assert(umi_fs_remove_tree(root) == UMI_STATUS_OK);
    assert(umi_fs_make_directories(root) == UMI_STATUS_OK);
    assert(umi_studio_services_create(NULL, NULL, &services) == UMI_STATUS_OK);
    assert(umi_studio_workspace_open(services, root, 1, 0) == UMI_STATUS_OK);
    assert(umi_studio_watcher_scan_once(services) == UMI_STATUS_OK);
    assert(umi_path_join(root, "watched.c", file, sizeof(file)) == UMI_STATUS_OK);
    assert(umi_fs_write_text(file, "int value;\n") == UMI_STATUS_OK);
    assert(umi_studio_watcher_scan_once(services) == UMI_STATUS_OK);

    watcher_stats = umi_studio_watcher_stats(services);
    index_stats = umi_file_index_stats(umi_studio_services_file_index(services));
    assert(watcher_stats.scans == 2U);
    assert(watcher_stats.created >= 1U);
    assert(index_stats.files == 1U);

    umi_studio_services_destroy(services);
    assert(umi_fs_remove_tree(root) == UMI_STATUS_OK);
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
