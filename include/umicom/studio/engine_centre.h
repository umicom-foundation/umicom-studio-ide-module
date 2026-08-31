/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/engine_centre.h
 *
 * PURPOSE:
 *   Project the Framework engine catalogue into a thin Studio panel and allow
 *   project templates to validate their reusable engine requirements.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_ENGINE_CENTRE_H
#define UMICOM_STUDIO_ENGINE_CENTRE_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Snapshot counts are derived from Framework and contain no Studio-owned list. */
typedef struct UmiStudioEngineCentreSnapshot {
    size_t engine_count;
    size_t contract_count;
    size_t foundation_count;
    size_t operational_count;
} UmiStudioEngineCentreSnapshot;

/* Count current engine maturity directly from the Framework catalogue. */
UmiStatus umi_studio_engine_centre_snapshot(
    UmiStudioEngineCentreSnapshot *out_snapshot);

/* Validate a new project's engine requirements using Framework rules. */
UmiStatus umi_studio_engine_centre_validate_project(
    const UmiEngineRequirement *requirements,
    size_t requirement_count,
    UmiEngineRequirementReport *out_report);

/* Build a toolkit-neutral Engine Explorer panel from Framework descriptors. */
UmiStatus umi_studio_engine_centre_view_create(
    const char *view_id,
    UmiUiViewModel **out_view);

#ifdef __cplusplus
}
#endif

#endif
