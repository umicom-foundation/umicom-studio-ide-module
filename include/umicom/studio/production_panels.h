/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/production_panels.h
 *
 * PURPOSE:
 *   Project the Framework-owned application production control plane into this
 *   thin product without duplicating layout, readiness or acceptance logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_PRODUCTION_PANELS_H
#define UMICOM_STUDIO_PRODUCTION_PANELS_H
#include "umicom/studio/production_profile.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UmiStudioProductionPanelContribution {
    const UmiApplicationProductionPanelBinding *binding;
    const char *workspace_area;
    int visible_by_default;
} UmiStudioProductionPanelContribution;
typedef struct UmiStudioProductionPanelCatalogue {
    UmiStudioProductionPanelContribution
        entries[UMI_APPLICATION_PRODUCTION_MAX_PANELS];
    size_t count;
    size_t critical_count;
    size_t context_linked_count;
} UmiStudioProductionPanelCatalogue;
/** Project runtime panel bindings into Studio workspace contributions. */
UmiStatus umi_studio_production_panels_build(
    const UmiApplicationProductionRuntime *runtime,
    UmiStudioProductionPanelCatalogue *out_catalogue);
#ifdef __cplusplus
}
#endif
#endif

