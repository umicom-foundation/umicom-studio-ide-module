/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/editor_intelligence_workbench_contribution.c
 *
 * PURPOSE:
 *   Place the Framework editor-intelligence runtime in Studio workbench regions
 *   and supply bounded Studio search-index capacity policy.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/editor_intelligence_workbench_contribution.h"

#include <string.h>

#define VIEW(identifier, label, contract, region, position, visible)          \
    {                                                                          \
        (uint32_t)sizeof(                                                      \
            UmiStudioEditorIntelligenceWorkbenchViewContribution),            \
        UMI_STUDIO_EDITOR_INTELLIGENCE_WORKBENCH_CONTRIBUTION_API_VERSION,     \
        identifier, label, contract, region, position, visible, 1, 1           \
    }

static const UmiStudioEditorIntelligenceWorkbenchViewContribution VIEWS[] = {
    VIEW("studio.editor-intelligence.search", "Workspace Search",
         "umicom.editor.intelligence-workbench.search-projection",
         "left-sidebar", 110, 1),
    VIEW("studio.editor-intelligence.navigation", "Source Insights",
         "umicom.editor.intelligence-workbench.navigation-projection",
         "secondary-sidebar", 120, 0),
    VIEW("studio.editor-intelligence.code-actions", "Code Actions",
         "umicom.editor.intelligence-workbench.code-action-projection",
         "secondary-sidebar", 130, 0),
    VIEW("studio.editor-intelligence.refactor-preview", "Refactor Preview",
         "umicom.editor.intelligence-workbench.refactor-projection",
         "bottom-panel", 140, 0),
    VIEW("studio.editor-intelligence.refactor-history", "Refactor History",
         "umicom.editor.intelligence-workbench.refactor-history",
         "bottom-panel", 150, 0)
};

#undef VIEW

/*
 * Return the number of records represented by studio editor intelligence workbench view
 * without changing their state.
 */
size_t umi_studio_editor_intelligence_workbench_view_count(void)
{
    return sizeof(VIEWS) / sizeof(VIEWS[0]);
}

/*
 * Find studio editor intelligence workbench view while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioEditorIntelligenceWorkbenchViewContribution *
umi_studio_editor_intelligence_workbench_view_at(size_t index)
{
    return index < umi_studio_editor_intelligence_workbench_view_count()
        ? &VIEWS[index]
        : NULL;
}

/*
 * Find studio editor intelligence workbench view while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioEditorIntelligenceWorkbenchViewContribution *
umi_studio_editor_intelligence_workbench_view_find(const char *view_id)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (view_id == NULL) return NULL;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < umi_studio_editor_intelligence_workbench_view_count();
         ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(VIEWS[index].view_id, view_id) == 0) return &VIEWS[index];
    }
    return NULL;
}

/*
 * Initialise studio editor intelligence workbench from caller-provided values so later
 * operations receive a known state.
 */
UmiStatus umi_studio_editor_intelligence_workbench_create(
    UmiEditorIntelWorkbenchRuntime **out_runtime)
{
    UmiEditorWorkspaceSearchIndexConfig config;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_runtime == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    umi_editor_workspace_search_index_config_init(&config);
    config.maximum_documents = 8192U;
    config.maximum_document_bytes = 16U * 1024U * 1024U;
    config.maximum_total_bytes = 512U * 1024U * 1024U;
    return umi_editor_intel_workbench_runtime_create(&config, out_runtime);
}
