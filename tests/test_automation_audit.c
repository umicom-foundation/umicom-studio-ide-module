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
#include "umicom/studio/automation_audit.h"
#include "umicom/studio/bootstrap.h"

#include <assert.h>

/* Start Studio headlessly and require every visible control reference to resolve. */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioAutomationAudit audit;

    (void)umi_fs_remove_tree(".umicom");
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
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
    return 0;
}
