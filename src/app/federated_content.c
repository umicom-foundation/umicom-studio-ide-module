/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/federated_content.c
 *
 * PURPOSE:
 *   Declare which existing Studio presentations fill Framework desktop
 *   windows. Framework continues to own hosting, lifecycle, context and all
 *   cross-application communication.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/federated_content.h"

#include <stdio.h>
#include <string.h>

#include "umicom/studio/workbench_views.h"

typedef struct UmiStudioFederatedContentDefinition {
    const char *window_id;
    const char *view_type;
    const char *view_id;
    UmiUiWindowGroupRole context_role;
} UmiStudioFederatedContentDefinition;

static const UmiStudioFederatedContentDefinition DEFINITIONS[] = {
    {
        "project-explorer",
        "studio.project-explorer",
        "studio.federated.project-explorer",
        UMI_UI_WINDOW_GROUP_SOURCE
    },
    {
        "output",
        "studio.output",
        "studio.federated.output",
        UMI_UI_WINDOW_GROUP_DESTINATION
    }
};

/*
 * Add studio federated content only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_federated_content_register(
    UmiDesktopContentRuntime *runtime,
    UmiStudioServices *services)
{
    size_t index;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || services == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_studio_workbench_views_register_registry(
        umi_desktop_content_runtime_view_factories(runtime), services);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < sizeof(DEFINITIONS) / sizeof(DEFINITIONS[0]);
         ++index) {
        UmiDesktopContentBinding binding;
        (void)memset(&binding, 0, sizeof(binding));
        binding.structure_size = (uint32_t)sizeof(binding);
        (void)snprintf(binding.window_id, sizeof(binding.window_id), "%s",
                       DEFINITIONS[index].window_id);
        (void)snprintf(binding.view_type, sizeof(binding.view_type), "%s",
                       DEFINITIONS[index].view_type);
        (void)snprintf(binding.view_id, sizeof(binding.view_id), "%s",
                       DEFINITIONS[index].view_id);
        binding.context_role = DEFINITIONS[index].context_role;
        status = umi_desktop_content_runtime_bind_window(runtime, &binding);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }
    return UMI_STATUS_OK;
}
