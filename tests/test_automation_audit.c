/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_automation_audit.c
 *
 * PURPOSE:
 *   Prevent visible Studio actions, menu rows and toolbar buttons from shipping
 *   with missing Framework command or action bindings.
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
#include "umicom/studio/automation_audit.h"
#include "umicom/studio/bootstrap.h"

#include <assert.h>

/* Start Studio headlessly and require every visible control reference to resolve. */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioAutomationAudit audit;

    /* Keep the former caller-relative cleanup for review. Each invocation now
     * owns a unique state directory, so direct execution cannot erase user state. */
#if 0
    (void)umi_fs_remove_tree(".umicom");
#endif
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
    assert(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);

    assert(umi_studio_automation_audit_capture(
        umi_studio_bootstrap_ui(bootstrap),
        &audit) == UMI_STATUS_OK);
    assert(audit.action_count >= audit.visible_action_count);
    assert(audit.visible_action_count == audit.wired_action_count);
    assert(umi_studio_automation_audit_validate(&audit) == UMI_STATUS_OK);

    assert(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
    umi_studio_bootstrap_destroy(bootstrap);
    /* Retain generated evidence instead of deleting a launch-directory tree. */
#if 0
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
#endif
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
