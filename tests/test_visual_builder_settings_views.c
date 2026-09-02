/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_visual_builder_settings_views.c
 *
 * PURPOSE:
 *   Verify the test visual builder settings views behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio Visual Builder v2 settings/views test. Sammy Hegab, Umicom Foundation, MIT. */
/* Validated settings and toolkit-neutral view models serve every frontend. */
#include "umicom/studio/visual_builder_settings.h"
#include "umicom/studio/visual_builder_views.h"
#include <assert.h>
#include <stdlib.h>
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){UmiSettings *settings=NULL;UmiStudioVisualBuilderConfig config;UmiStudioVisualBuilderCentre *centre=NULL;UmiUiViewModel *view=NULL;UmiUiViewSnapshot snapshot;assert(umi_studio_settings_create(&settings)==UMI_STATUS_OK);config=umi_studio_visual_builder_config_default();assert(umi_studio_visual_builder_settings_apply(settings,&config)==UMI_STATUS_OK);assert(config.surface.grid_size==8U);assert(umi_studio_visual_builder_centre_create("org.umicom.views",&centre)==UMI_STATUS_OK);assert(umi_studio_visual_builder_view_create(centre,UMI_STUDIO_VISUAL_BUILDER_PANE_SOURCE,&view)==UMI_STATUS_OK);assert(umi_ui_view_model_snapshot(view,&snapshot)==UMI_STATUS_OK);assert(snapshot.role==UMI_UI_ROLE_PANE);umi_ui_view_model_destroy(view);umi_studio_visual_builder_centre_destroy(centre);umi_settings_destroy(settings);return EXIT_SUCCESS;}
