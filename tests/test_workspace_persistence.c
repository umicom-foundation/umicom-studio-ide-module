/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_workspace_persistence.c
 *
 * PURPOSE:
 *   Verify the test workspace persistence behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>
#include "operations_workspace_fixture.h"
#include "umicom/studio/workspace_catalogue.h"
#include "umicom/studio/workspace_groups.h"
#include "umicom/studio/workspace_persistence.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioProfessionalWorkspace *source = umi_test_seeded_professional_workspace();
    UmiStudioProfessionalWorkspace *target = umi_test_professional_workspace();
    UmiUiWorkspaceCustomisation *target_model = umi_studio_professional_workspace_model(target);
    UmiUiWorkspaceImportReport report;
    char encoded[UMI_UI_LAYOUT_ENCODED_CAPACITY];
    /* Import validation needs the receiving Studio catalogue and context
     * groups, but it does not need pre-existing layouts. */
    assert(umi_studio_workspace_catalogue_seed(target) == UMI_STATUS_OK);
    assert(umi_studio_workspace_groups_seed(target) == UMI_STATUS_OK);
    assert(umi_studio_workspace_execute(source,UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_COMPARE) == UMI_STATUS_OK);
    assert(umi_studio_workspace_persistence_export_active(source,5700U,encoded,sizeof(encoded)) == UMI_STATUS_OK);
    assert(strstr(encoded,"UMILAYOUT3") != NULL);
    assert(umi_studio_workspace_persistence_import(target,encoded,true) == UMI_STATUS_OK);
    assert(target_model->layout_count == 1U);
    assert(strcmp(target_model->active_layout_id,"compare") == 0);
    assert(target_model->layouts[0].window_count == 4U);
    /* Re-import must be an explicit replacement so a user cannot accidentally
     * overwrite a named Studio layout. */
    assert(umi_studio_workspace_persistence_import(target,encoded,true) ==
           UMI_STATUS_ALREADY_EXISTS);
    assert(umi_studio_workspace_persistence_import_with_report(
               target, encoded, true, true, &report) == UMI_STATUS_OK);
    assert(report.replaced);
    assert(strcmp(report.layout_id,"compare") == 0);
    umi_studio_professional_workspace_destroy(target);
    umi_studio_professional_workspace_destroy(source);
    return 0;
}
