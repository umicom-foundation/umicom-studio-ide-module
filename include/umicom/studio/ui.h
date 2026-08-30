/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/ui.h
 *
 * PURPOSE:
 *   Own the toolkit-neutral Studio UI composition built from Umicom Framework
 *   workbench, application-shell, headless-adapter and command contracts.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_UI_H
#define UMICOM_STUDIO_UI_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/umicom.h"
#include "umicom/studio/application_surface.h"
#include "umicom/studio/application_surface_policy.h"
#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UmiStudioUi UmiStudioUi;
typedef struct UmiStudioViewModels UmiStudioViewModels;

typedef struct UmiStudioUiSnapshot {
    size_t perspectives;
    size_t panes;
    size_t documents;
    size_t actions;
    size_t menus;
    size_t toolbars;
    size_t status_items;
    size_t contributions;
    size_t notifications;
    size_t application_panels;
    size_t visible_application_panels;
    size_t ready_application_panels;
    size_t application_panels_needing_attention;
    size_t scheduled_application_panels;
    size_t streaming_application_panels;
    size_t guarded_application_panels;
    size_t context_enabled_application_panels;
    uint32_t checkpoint_interval_seconds;
    uint64_t workbench_revision;
    uint64_t render_revision;
    uint64_t application_surface_revision;
    char active_perspective[UMI_UI_ID_CAPACITY];
    char active_document[UMI_UI_ID_CAPACITY];
    char active_application_component[UMI_UI_ID_CAPACITY];
} UmiStudioUiSnapshot;

UmiStatus umi_studio_ui_create(UmiStudioServices *services,
                               UmiCommandRegistry *commands,
                               UmiStudioUi **out_ui);
void umi_studio_ui_destroy(UmiStudioUi *ui);
UmiStatus umi_studio_ui_publish(UmiStudioUi *ui,
                                UmiServiceRegistry *registry);
UmiStatus umi_studio_ui_refresh(UmiStudioUi *ui);
UmiStatus umi_studio_ui_advance(UmiStudioUi *ui, uint32_t elapsed_seconds);
UmiStatus umi_studio_ui_set_background(UmiStudioUi *ui, int background);
UmiStatus umi_studio_ui_context_changed(UmiStudioUi *ui,
                                        const char *component_id,
                                        const char *context_value);
int umi_studio_ui_checkpoint_due(const UmiStudioUi *ui,
                                 uint32_t elapsed_seconds,
                                 int changed);
UmiStatus umi_studio_ui_snapshot(const UmiStudioUi *ui,
                                 UmiStudioUiSnapshot *out_snapshot);
UmiStatus umi_studio_ui_render_headless(UmiStudioUi *ui,
                                        UmiUiHeadlessSnapshot *out_snapshot);

UmiStudioServices *umi_studio_ui_services(UmiStudioUi *ui);
UmiUiApplicationShell *umi_studio_ui_shell(UmiStudioUi *ui);
UmiUiWorkbench *umi_studio_ui_workbench(UmiStudioUi *ui);
UmiUiHeadlessAdapter *umi_studio_ui_headless(UmiStudioUi *ui);
UmiStudioViewModels *umi_studio_ui_view_models(UmiStudioUi *ui);
UmiDocumentCoordinator *umi_studio_ui_documents(UmiStudioUi *ui);
UmiStudioApplicationSurface *umi_studio_ui_application_surface(UmiStudioUi *ui);

#ifdef __cplusplus
}
#endif

#endif
