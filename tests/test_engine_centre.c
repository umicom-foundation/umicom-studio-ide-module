/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_engine_centre.c
 *
 * PURPOSE:
 *   Verify Studio projects and the Engine Explorer consume Framework truth.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>

#include "umicom/studio/engine_centre.h"

/* Confirm the thin Studio layer does not maintain a second engine catalogue. */
int main(void)
{
    UmiStudioEngineCentreSnapshot snapshot;
    UmiEngineRequirement requirement = {
        UMI_ENGINE_RENDER_3D,
        UMI_ENGINE_MATURITY_FOUNDATION,
        UMI_ENGINE_CAPABILITY_SCENE_GRAPH};
    UmiEngineRequirementReport report;
    UmiUiViewModel *view = NULL;

    /* Snapshot total must always equal the canonical Framework catalogue count. */
    assert(umi_studio_engine_centre_snapshot(&snapshot) == UMI_STATUS_OK);
    assert(snapshot.engine_count == umi_engine_catalogue_count());
    assert(snapshot.operational_count > 0U);
    assert(snapshot.foundation_count > 0U);
    assert(umi_studio_engine_centre_validate_project(
        &requirement, 1U, &report) == UMI_STATUS_OK);
    assert(report.ready);

    /* View construction proves the catalogue is available to any GTK adapter. */
    assert(umi_studio_engine_centre_view_create(
        "studio.engine-explorer", &view) == UMI_STATUS_OK);
    assert(view != NULL);
    umi_ui_view_model_destroy(view);
    return 0;
}
