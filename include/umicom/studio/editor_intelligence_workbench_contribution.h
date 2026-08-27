/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/editor_intelligence_workbench_contribution.h
 *
 * PURPOSE:
 *   Declare Studio placement and composition for the Framework-owned
 *   operational editor-intelligence workbench.
 *
 * ARCHITECTURE:
 *   This application layer contains no search, navigation, language or edit
 *   semantics. It selects Studio view regions and creates the public Framework
 *   runtime with Studio capacity policy.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_EDITOR_INTELLIGENCE_WORKBENCH_CONTRIBUTION_H
#define UMICOM_STUDIO_EDITOR_INTELLIGENCE_WORKBENCH_CONTRIBUTION_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/editor/intelligence_workbench/runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_EDITOR_INTELLIGENCE_WORKBENCH_CONTRIBUTION_API_VERSION 1U

typedef struct UmiStudioEditorIntelligenceWorkbenchViewContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *view_id;
    const char *title;
    const char *framework_contract;
    const char *default_region;
    int32_t order;
    int visible_by_default;
    int closable;
    int movable;
} UmiStudioEditorIntelligenceWorkbenchViewContribution;

size_t umi_studio_editor_intelligence_workbench_view_count(void);
const UmiStudioEditorIntelligenceWorkbenchViewContribution *
umi_studio_editor_intelligence_workbench_view_at(size_t index);
const UmiStudioEditorIntelligenceWorkbenchViewContribution *
umi_studio_editor_intelligence_workbench_view_find(const char *view_id);

UmiStatus umi_studio_editor_intelligence_workbench_create(
    UmiEditorIntelWorkbenchRuntime **out_runtime);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_EDITOR_INTELLIGENCE_WORKBENCH_CONTRIBUTION_H */
