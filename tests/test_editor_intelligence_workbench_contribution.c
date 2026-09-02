/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_editor_intelligence_workbench_contribution.c
 *
 * PURPOSE:
 *   Verify Studio contributes only unique placements and composes the public
 *   Framework editor-intelligence runtime.
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

#include "umicom/studio/editor_intelligence_workbench_contribution.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiEditorIntelWorkbenchRuntime *runtime = NULL;
    UmiEditorIntelWorkbenchSnapshot snapshot;
    size_t index;
    size_t comparison;

    assert(umi_studio_editor_intelligence_workbench_view_count() == 5U);
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < umi_studio_editor_intelligence_workbench_view_count();
         ++index) {
        const UmiStudioEditorIntelligenceWorkbenchViewContribution *view =
            umi_studio_editor_intelligence_workbench_view_at(index);
        assert(view != NULL);
        assert(view->struct_size == (uint32_t)sizeof(*view));
        assert(view->api_version ==
               UMI_STUDIO_EDITOR_INTELLIGENCE_WORKBENCH_CONTRIBUTION_API_VERSION);
        assert(view->view_id[0] != '\0');
        assert(view->title[0] != '\0');
        assert(view->framework_contract[0] != '\0');
        assert(view->default_region[0] != '\0');
        assert(view->closable);
        assert(view->movable);
        /* Visit each bounded item once so every record receives the same rule. */
        for (comparison = index + 1U;
             comparison < umi_studio_editor_intelligence_workbench_view_count();
             ++comparison) {
            assert(strcmp(
                       view->view_id,
                       umi_studio_editor_intelligence_workbench_view_at(
                           comparison)->view_id) != 0);
        }
    }
    assert(umi_studio_editor_intelligence_workbench_view_find(
               "studio.editor-intelligence.code-actions") != NULL);
    assert(umi_studio_editor_intelligence_workbench_view_at(5U) == NULL);
    assert(umi_studio_editor_intelligence_workbench_create(&runtime) ==
           UMI_STATUS_OK);
    assert(umi_editor_intel_workbench_runtime_snapshot(runtime, &snapshot) ==
           UMI_STATUS_OK);
    assert(snapshot.search_result_count == 0U);
    assert(snapshot.navigation_result_count == 0U);
    assert(snapshot.refactor_phase == UMI_EDITOR_INTEL_PHASE_IDLE);
    umi_editor_intel_workbench_runtime_destroy(runtime);
    return 0;
}
