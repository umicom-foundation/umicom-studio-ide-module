/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/production_readiness.h
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
#ifndef UMICOM_STUDIO_PRODUCTION_READINESS_H
#define UMICOM_STUDIO_PRODUCTION_READINESS_H
#include "umicom/studio/production_profile.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UmiStudioProductionReadiness {
    UmiApplicationProductionState state;
    size_t panel_count;
    size_t layout_count;
    size_t open_framework_feature_count;
    size_t open_application_feature_count;
    size_t open_adapter_feature_count;
    unsigned readiness_percent;
    int launchable;
} UmiStudioProductionReadiness;
/** Summarise Studio launch readiness from the shared production runtime. */
UmiStatus umi_studio_production_readiness_build(
    const UmiApplicationProductionRuntime *runtime,
    UmiStudioProductionReadiness *out_readiness);
#ifdef __cplusplus
}
#endif
#endif

