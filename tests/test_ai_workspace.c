/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Tests
 * File: applications/studio/tests/test_ai_workspace.c
 *
 * PURPOSE:
 *   Verify the Studio AI Workspace reports live AuthorEngine integration state.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Setup and contract assertions remain active in optimized test builds. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "studio_test_state.h"
#include <assert.h>
#include <string.h>

#include "umicom/studio/ai_workspace.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioServices *services = NULL;
    UmiStudioAiWorkspaceSnapshot snapshot;
    /* Keep the former caller-relative cleanup for review. Each invocation now
     * owns a unique state directory, so direct execution cannot erase user state. */
#if 0
    (void)umi_fs_remove_tree(".umicom");
#endif
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
    assert(umi_studio_services_create(NULL, NULL, &services) == UMI_STATUS_OK);
    assert(umi_studio_ai_workspace_snapshot(services, &snapshot)
        == UMI_STATUS_OK);
    assert(snapshot.available);
    assert(snapshot.runtime_count == 3U);
    assert(snapshot.context_source_count == 3U);
    assert(snapshot.session_count == 1U);
    assert(snapshot.item_count == 7U);
    assert(strcmp(snapshot.title, "AI Workspace") == 0);
    assert(strcmp(snapshot.active_session_id, "studio.session.default") == 0);
    umi_studio_services_destroy(services);
    /* Retain generated evidence instead of deleting a launch-directory tree. */
#if 0
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
#endif
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
