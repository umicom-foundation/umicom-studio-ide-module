/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_data_workbench_contribution.c
 *
 * PURPOSE:
 *   Verify unique Studio placements and direct composition of the Framework
 *   Database workbench runtime.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "umicom/studio/data_workbench_contribution.h"

int main(void)
{
    UmiDataServer *server = NULL;
    UmiDatabaseExplorer *explorer = NULL;
    UmiDataWorkbenchRuntime *runtime = NULL;
    UmiDataWorkbenchRuntimeSnapshot snapshot;
    size_t index;
    size_t comparison;
    assert(umi_studio_data_workbench_command_count() ==
           umi_data_workbench_command_count());
    for (index = 0U; index < umi_studio_data_workbench_command_count(); ++index) {
        assert(umi_data_workbench_command_find(
                   umi_studio_data_workbench_command_at(index)->
                       framework_command_id) != NULL);
    }
    assert(umi_studio_data_workbench_view_count() == 11U);
    for (index = 0U; index < umi_studio_data_workbench_view_count(); ++index) {
        const UmiStudioDataWorkbenchViewContribution *view =
            umi_studio_data_workbench_view_at(index);
        assert(view != NULL && view->framework_contract[0] != '\0');
        for (comparison = index + 1U;
             comparison < umi_studio_data_workbench_view_count(); ++comparison) {
            assert(strcmp(
                       view->view_id,
                       umi_studio_data_workbench_view_at(comparison)->view_id) !=
                   0);
        }
    }
    assert(umi_data_server_create_memory(&server) == UMI_STATUS_OK);
    explorer = (UmiDatabaseExplorer *)calloc(1U, sizeof(*explorer));
    assert(explorer != NULL);
    assert(umi_database_explorer_init(explorer, server) == UMI_STATUS_OK);
    assert(umi_studio_data_workbench_create(explorer, &runtime) ==
           UMI_STATUS_OK);
    assert(umi_data_workbench_runtime_snapshot(runtime, &snapshot) ==
           UMI_STATUS_OK);
    assert(snapshot.phase == UMI_DATA_WORKBENCH_IDLE);
    umi_data_workbench_runtime_destroy(runtime);
    free(explorer);
    umi_data_server_destroy(server);
    return 0;
}
