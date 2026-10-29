/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Tests
 * File: applications/studio/tests/test_extension_centre.c
 *
 * PURPOSE:
 *   Verify live Extension Centre snapshot and toolkit-neutral workbench views.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE:
 *   MIT
 *----------------------------------------------------------------------------*/
/* Setup and contract assertions remain active in optimized test builds. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "studio_test_state.h"
#include <assert.h>

#include "umicom/studio/bootstrap.h"
#include "umicom/studio/extension_centre.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioServices *services;
    UmiStudioExtensionCentreSnapshot snapshot;
    UmiUiViewModel *view = NULL;
    UmiUiValue value;
    /* Keep the former caller-relative cleanup for review. Each invocation now
     * owns a unique state directory, so direct execution cannot erase user state. */
#if 0
    (void)umi_fs_remove_tree(".umicom");
#endif
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
    assert(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    services = umi_studio_bootstrap_services(bootstrap);
    assert(umi_studio_extension_centre_snapshot(services, &snapshot) == UMI_STATUS_OK);
    assert(snapshot.available == 1);
    assert(snapshot.api_version == 2U);
    assert(snapshot.extension_points == 8U);
    assert(snapshot.catalogue_entries == 3U);
    assert(umi_studio_extension_centre_installed_view(
        services, "studio.test.extensions", &view) == UMI_STATUS_OK);
    assert(umi_ui_view_model_get_property(view, "extensions.installed", &value) == UMI_STATUS_OK);
    assert(value.kind == UMI_UI_VALUE_INTEGER);
    umi_ui_view_model_destroy(view);
    view = NULL;
    assert(umi_studio_extension_centre_catalogue_view(
        services, "studio.test.catalogue", &view) == UMI_STATUS_OK);
    assert(umi_ui_view_model_get_property(view, "catalogue.row-count", &value) == UMI_STATUS_OK);
    assert(value.integer_value == 3);
    umi_ui_view_model_destroy(view);
    umi_studio_bootstrap_destroy(bootstrap);
    /* Retain generated evidence instead of deleting a launch-directory tree. */
#if 0
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
#endif
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
