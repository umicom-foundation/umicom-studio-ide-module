/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_workspace_themes.c
 *
 * PURPOSE:
 *   Verify the test workspace themes behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* This focused test checks that Studio receives its theme colours and density
 * from the shared Framework appearance model. */
#include <assert.h>
#include <string.h>
#include "operations_workspace_fixture.h"
#include "umicom/ui/brand_palette.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioProfessionalWorkspace *workspace = umi_test_seeded_professional_workspace();
    UmiUiWorkspaceCustomisation *model = umi_studio_professional_workspace_model(workspace);
    const UmiUiBrandPalette *palette =
        umi_ui_brand_palette_get(UMI_UI_BRAND_SURFACE_HIGH_CONTRAST);
    assert(palette != NULL);
    assert(umi_studio_workspace_theme_apply(workspace,UMI_STUDIO_WORKSPACE_THEME_HIGH_CONTRAST,UMI_UI_DENSITY_COMPACT,1.25) == UMI_STATUS_OK);
    assert(model->theme.mode == UMI_UI_THEME_MODE_HIGH_CONTRAST);
    assert(model->theme.font_scale == 1.25);
    /* Theme projection uses Framework's canonical palette rather than a
     * copied literal whose letter case can drift from the shared source. */
    assert(strcmp(model->theme.accent, palette->accent) == 0);
    umi_studio_professional_workspace_destroy(workspace);
    return 0;
}
