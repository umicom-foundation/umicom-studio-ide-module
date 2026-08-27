/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_developer_ecosystem_contribution.c
 * PURPOSE: Verify complete command alignment and unique thin Studio placements.
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>

#include "umicom/studio/developer_ecosystem_contribution.h"

int main(void)
{
    UmiEcosystemRuntime *runtime = NULL;
    size_t index;
    assert(umi_studio_developer_ecosystem_command_count() ==
        umi_ecosystem_command_count());
    for (index = 0U; index < umi_studio_developer_ecosystem_command_count(); ++index) {
        const UmiStudioDeveloperEcosystemCommandContribution *item =
            umi_studio_developer_ecosystem_command_at(index);
        assert(item != NULL && item->enabled);
        assert(umi_ecosystem_command_find(item->framework_command_id) != NULL);
        assert(umi_studio_developer_ecosystem_command_find(
            item->framework_command_id) == item);
    }
    assert(umi_studio_developer_ecosystem_view_count() == 12U);
    for (index = 0U; index < umi_studio_developer_ecosystem_view_count(); ++index) {
        const UmiStudioDeveloperEcosystemViewContribution *view =
            umi_studio_developer_ecosystem_view_at(index);
        size_t other;
        assert(view != NULL && view->visible && view->closable);
        assert(strncmp(view->framework_contract, "umicom.", 7U) == 0);
        for (other = index + 1U;
             other < umi_studio_developer_ecosystem_view_count(); ++other) {
            assert(strcmp(view->view_id,
                umi_studio_developer_ecosystem_view_at(other)->view_id) != 0);
        }
    }
    assert(umi_studio_developer_ecosystem_create(&runtime) == UMI_STATUS_OK);
    assert(runtime != NULL);
    umi_ecosystem_runtime_destroy(runtime);
    return 0;
}
