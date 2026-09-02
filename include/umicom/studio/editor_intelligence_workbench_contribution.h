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
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
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

/**
 * Represent the studio editor intelligence workbench view contribution data shared with
 * callers of this public contract.
 */
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

/**
 * Return the number of records represented by studio editor intelligence workbench view
 * without changing their state.
 */
size_t umi_studio_editor_intelligence_workbench_view_count(void);
/**
 * Find studio editor intelligence workbench view while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioEditorIntelligenceWorkbenchViewContribution *
umi_studio_editor_intelligence_workbench_view_at(size_t index);
/**
 * Find studio editor intelligence workbench view while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioEditorIntelligenceWorkbenchViewContribution *
umi_studio_editor_intelligence_workbench_view_find(const char *view_id);

/**
 * Initialise studio editor intelligence workbench from caller-provided values so later
 * operations receive a known state.
 */
UmiStatus umi_studio_editor_intelligence_workbench_create(
    UmiEditorIntelWorkbenchRuntime **out_runtime);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_EDITOR_INTELLIGENCE_WORKBENCH_CONTRIBUTION_H */
