/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/workspace_main.c
 *
 * PURPOSE:
 *   Inspect an indexed Studio workspace, discovered projects and optional text
 *   search from a native command without starting GTK4.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/workspace.h"
#include "umicom/studio/search.h"

#include <stdio.h>
#include <string.h>

/* Provide the print match operation used by this module and its client applications. */
static UmiStatus print_match(const UmiSearchMatch *match, void *user_data)
{
    (void)user_data;
    (void)printf("%s:%zu:%zu: %s\n",
                 match->path,
                 match->line,
                 match->column,
                 match->preview);
    return UMI_STATUS_OK;
}

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc, char **argv)
{
    UmiStudioServices *services = NULL;
    UmiStudioWorkspaceSnapshot snapshot;
    UmiStatus status;
    const char *root = argc >= 2 ? argv[1] : ".";
    const char *query = NULL;
    size_t index;
    int trusted = 0;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 2U; index < (size_t)argc; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(argv[index], "--trusted") == 0) trusted = 1;
        else /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (strcmp(argv[index], "--search") == 0 && index + 1U < (size_t)argc) {
            query = argv[++index];
        }
    }

    status = umi_studio_services_create(NULL, NULL, &services);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_workspace_open(services, root, trusted, 0);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_workspace_snapshot(services, &snapshot);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr, "Workspace failed: %s\n", umi_status_text(status));
        umi_studio_services_destroy(services);
        return 1;
    }

    (void)printf("Workspace: %s\n", snapshot.graph.root);
    (void)printf("Trusted: %s\n", snapshot.graph.trusted ? "yes" : "no");
    (void)printf("Projects: %zu\n", snapshot.graph.project_count);
    (void)printf("Indexed files: %zu\n", snapshot.files.files);
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < snapshot.graph.project_count; ++index) {
        UmiWorkspaceProjectSnapshot project;
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (umi_studio_workspace_project_at(services, index, &project) ==
            UMI_STATUS_OK) {
            (void)printf("Project[%zu]: %s (%s)\n",
                         index,
                         project.display_name,
                         project.root);
        }
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (query != NULL) {
        UmiSearchRequest request = umi_search_request_default(query);
        UmiSearchStats stats;
        request.maximum_results = 50U;
        status = umi_studio_search_text(services,
                                        &request,
                                        print_match,
                                        NULL,
                                        &stats);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            (void)printf("Matches: %zu; files searched: %zu\n",
                         stats.matches,
                         stats.files_searched);
        }
    }

    umi_studio_services_destroy(services);
    return status == UMI_STATUS_OK ? 0 : 1;
}
