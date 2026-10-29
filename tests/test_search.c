/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_search.c
 *
 * PURPOSE:
 *   Verify file-name and bounded text search through Studio's shared Framework
 *   workspace index.
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
#include "umicom/studio/search.h"
#include "umicom/studio/workspace.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
 * Exercise count match and return a clear result when the behaviour no longer matches its
 * contract.
 */
static UmiStatus count_match(const UmiSearchMatch *match, void *user_data)
{
    size_t *count = (size_t *)user_data;
    assert(match != NULL);
    assert(strstr(match->preview, "needle") != NULL);
    *count += 1U;
    return UMI_STATUS_OK;
}

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
    UmiFileIndexEntry entries[4];
    UmiSearchRequest request;
    UmiSearchStats stats;
    char temporary[UMI_PATH_CAPACITY];
    char root[UMI_PATH_CAPACITY];
    char first[UMI_PATH_CAPACITY];
    char second[UMI_PATH_CAPACITY];
    size_t count = 0U;
    size_t matches = 0U;

    /* Fixed fixture names belong beneath this invocation, not the shared temp root.
     * Retain the old base selection for review of the ownership correction. */
#if 0
    assert(umi_fs_temp_directory(temporary, sizeof(temporary)) == UMI_STATUS_OK);
#endif
    assert(umi_path_copy(temporary, sizeof(temporary), test_state.directory) == UMI_STATUS_OK);
    assert(umi_path_join(temporary,
                         "umicom-studio-search-test",
                         root,
                         sizeof(root)) == UMI_STATUS_OK);
    assert(umi_fs_remove_tree(root) == UMI_STATUS_OK);
    assert(umi_fs_make_directories(root) == UMI_STATUS_OK);
    assert(umi_path_join(root, "first.c", first, sizeof(first)) == UMI_STATUS_OK);
    assert(umi_path_join(root, "second.txt", second, sizeof(second)) ==
           UMI_STATUS_OK);
    assert(umi_fs_write_text(first, "one needle two\n") == UMI_STATUS_OK);
    assert(umi_fs_write_text(second, "nothing here\n") == UMI_STATUS_OK);
    assert(umi_studio_services_create(NULL, NULL, &services) == UMI_STATUS_OK);
    assert(umi_studio_workspace_open(services, root, 0, 0) == UMI_STATUS_OK);

    assert(umi_studio_search_files(services,
                                   "first",
                                   0,
                                   entries,
                                   4U,
                                   &count) == UMI_STATUS_OK);
    assert(count == 1U);
    request = umi_search_request_default("needle");
    request.maximum_results = 4U;
    assert(umi_studio_search_text(services,
                                  &request,
                                  count_match,
                                  &matches,
                                  &stats) == UMI_STATUS_OK);
    assert(matches == 1U);
    assert(stats.matches == 1U);

    umi_studio_services_destroy(services);
    assert(umi_fs_remove_tree(root) == UMI_STATUS_OK);
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
