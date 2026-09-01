/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_workspace_layouts.c
 *
 * PURPOSE:
 *   Verify the test workspace layouts behavior for
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
int main(void)
{
    UmiStudioProfessionalWorkspace *workspace = umi_test_seeded_professional_workspace();
    UmiUiWorkspaceCustomisation *model = umi_studio_professional_workspace_model(workspace);
    UmiUiWorkspaceCustomisationSnapshot customisation_snapshot;
    UmiUiWorkspaceLayout *active;
    assert(model->layout_count == 16U && model->library.count == 16U);
    assert(umi_ui_layout_library_find(&model->library,"studio-focus") != NULL);
    assert(umi_ui_layout_library_find(&model->library,"studio-debug") != NULL);
    assert(umi_ui_layout_library_find(&model->library,"studio-trading") != NULL);
    assert(umi_ui_layout_library_find(&model->library,"studio-treasury") != NULL);
    assert(umi_ui_layout_library_find(&model->library,"studio-media") != NULL);
    assert(umi_ui_layout_library_find(&model->library,"studio-music") != NULL);
    assert(umi_ui_layout_library_find(&model->library,"studio-creator") != NULL);
    assert(umi_ui_layout_library_find(&model->library,"studio-knowledge") != NULL);
    assert(umi_ui_layout_library_find(&model->library,"studio-model-lab") != NULL);
    assert(umi_ui_layout_library_find(&model->library,"studio-games") != NULL);
    assert(umi_ui_layout_library_find(&model->library,"studio-cad") != NULL);
    assert(umi_ui_layout_library_find(&model->library,"studio-kitchen") != NULL);
    assert(umi_ui_layout_library_find(&model->library,"studio-author") != NULL);
    assert(umi_studio_workspace_execute(
               workspace, UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_TRADING) ==
           UMI_STATUS_OK);
    active = umi_ui_workspace_customisation_active(model);
    assert(active != NULL && active->window_count == 5U && active->locked);
    assert(umi_studio_workspace_execute(workspace,UMI_STUDIO_WORKSPACE_COMMAND_ACTIVATE_COMPARE) == UMI_STATUS_OK);
    active = umi_ui_workspace_customisation_active(model);
    assert(active != NULL && strcmp(active->layout_id,"compare") == 0);
    assert(active->window_count == 4U && active->locked);
    assert(umi_studio_workspace_execute(workspace,UMI_STUDIO_WORKSPACE_COMMAND_UNLOCK) == UMI_STATUS_OK);
    assert(!active->locked);
    {
        UmiUiWorkspacePanelSettings settings =
            umi_ui_workspace_panel_settings_default(
                active->windows[0].window_id);
        settings.placement_id = "right";
        settings.stack_id = "studio-inspector";
        settings.context_group_id = "compare-orange";
        /* The Studio wrapper must leave transaction semantics in Framework. */
        assert(umi_studio_workspace_apply_panel_settings(
                   workspace, &settings) == UMI_STATUS_OK);
    }
    umi_ui_workspace_customisation_snapshot(model, &customisation_snapshot);
    assert(customisation_snapshot.editing);
    assert(umi_studio_workspace_execute(workspace,UMI_STUDIO_WORKSPACE_COMMAND_LOCK) == UMI_STATUS_OK);
    umi_ui_workspace_customisation_snapshot(model, &customisation_snapshot);
    assert(active->locked && !customisation_snapshot.editing);
    umi_studio_professional_workspace_destroy(workspace);
    return 0;
}
