/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_workspace.c
 *
 * PURPOSE:
 *   Verify Studio workspace opening, project discovery, indexing, trust and
 *   watcher lifecycle through shared Framework services.
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
#include "umicom/studio/workspace.h"
#include "umicom/studio/watcher.h"

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
    UmiStudioWorkspaceSnapshot snapshot;
    UmiWorkspaceProjectSnapshot project;
    char temporary[UMI_PATH_CAPACITY];
    char root[UMI_PATH_CAPACITY];
    char project_root[UMI_PATH_CAPACITY];
    char cmake[UMI_PATH_CAPACITY];
    char source[UMI_PATH_CAPACITY];

    /* Fixed fixture names belong beneath this invocation, not the shared temp root.
     * Retain the old base selection for review of the ownership correction. */
#if 0
    assert(umi_fs_temp_directory(temporary, sizeof(temporary)) == UMI_STATUS_OK);
#endif
    assert(umi_path_copy(temporary, sizeof(temporary), test_state.directory) == UMI_STATUS_OK);
    assert(umi_path_join(temporary,
                         "umicom-studio-workspace-test",
                         root,
                         sizeof(root)) == UMI_STATUS_OK);
    assert(umi_fs_remove_tree(root) == UMI_STATUS_OK);
    assert(umi_path_join(root, "sample", project_root, sizeof(project_root)) ==
           UMI_STATUS_OK);
    assert(umi_fs_make_directories(project_root) == UMI_STATUS_OK);
    assert(umi_path_join(project_root, "CMakeLists.txt", cmake, sizeof(cmake)) ==
           UMI_STATUS_OK);
    assert(umi_path_join(project_root, "main.c", source, sizeof(source)) ==
           UMI_STATUS_OK);
    assert(umi_fs_write_text(cmake, "project(Sample C)\n") == UMI_STATUS_OK);
    assert(umi_fs_write_text(source, "int main(void){return 0;}\n") ==
           UMI_STATUS_OK);

    assert(umi_studio_services_create(NULL, NULL, &services) == UMI_STATUS_OK);
    assert(umi_studio_workspace_open(services, root, 1, 0) == UMI_STATUS_OK);
    assert(umi_studio_workspace_snapshot(services, &snapshot) == UMI_STATUS_OK);
    assert(snapshot.graph.open);
    assert(snapshot.graph.trusted);
    assert(snapshot.graph.project_count == 1U);
    assert(snapshot.files.files == 2U);
    assert(umi_studio_workspace_project_at(services, 0U, &project) ==
           UMI_STATUS_OK);
    assert(project.kind == UMI_PROJECT_CMAKE);
    assert(umi_studio_watcher_start(services) == UMI_STATUS_OK);
    assert(umi_studio_watcher_stop(services) == UMI_STATUS_OK);
    assert(umi_studio_workspace_close(services) == UMI_STATUS_OK);

    umi_studio_services_destroy(services);
    assert(umi_fs_remove_tree(root) == UMI_STATUS_OK);
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
