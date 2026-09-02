/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/production_panels.c
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
#include "umicom/studio/production_panels.h"
#include <string.h>
/*
 * Provide the studio production panels build operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_production_panels_build(
    const UmiApplicationProductionRuntime *runtime,
    UmiStudioProductionPanelCatalogue *out_catalogue)
{
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || out_catalogue == NULL || !runtime->initialised ||
        strcmp(runtime->binding.experience->application_id,
               "org.umicom.studio") != 0)
        return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_catalogue, 0, sizeof(*out_catalogue));
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < runtime->panels.count; ++index) {
        const UmiApplicationProductionPanelBinding *binding =
            &runtime->panels.entries[index];
        UmiStudioProductionPanelContribution *entry =
            &out_catalogue->entries[out_catalogue->count];
        entry->binding = binding;
        entry->workspace_area =
            binding->panel->default_region != NULL
            ? binding->panel->default_region : "centre";
        entry->visible_by_default =
            umi_application_session_panel_active(
                &runtime->workspace.session, binding->panel->panel_id);
        out_catalogue->critical_count += (size_t)binding->required;
        out_catalogue->context_linked_count +=
            (size_t)((binding->panel->flags &
                      UMI_EXPERIENCE_PANEL_CONTEXT_LINKED) != 0U);
        out_catalogue->count += 1U;
    }
    return UMI_STATUS_OK;
}

