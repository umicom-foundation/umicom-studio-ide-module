/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/workspace_layouts.c
 *
 * PURPOSE:
 *   Adapt the Framework-owned professional desktop layout catalogue to the
 *   Studio semantic workspace model. Studio contains no private layout logic;
 *   every definition and reusable behaviour comes from Umicom Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/workspace_layouts.h"

#include <string.h>

#include "umicom/application/experience_portfolio.h"
#include "umicom/desktop/desktop.h"
#include "umicom/studio/contributions.h"

/*
 * Provide the studio pane resolver operation used by this module and its client
 * applications.
 */
static const char *studio_pane_resolver(
    const UmiExperiencePanelDefinition *panel,
    void *user_data)
{
    (void)user_data;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || panel->panel_id == NULL) return NULL;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "resource-explorer") == 0)
        return UMI_STUDIO_PANE_EXPLORER;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "editor") == 0)
        return UMI_STUDIO_PANE_EDITOR;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "problems") == 0)
        return UMI_STUDIO_PANE_PROBLEMS;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "output") == 0)
        return UMI_STUDIO_PANE_OUTPUT;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "terminal") == 0)
        return UMI_STUDIO_PANE_TERMINAL;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "source-control") == 0)
        return UMI_STUDIO_PANE_SOURCE_CONTROL;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "test-explorer") == 0)
        return UMI_STUDIO_PANE_TESTING;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "debug") == 0)
        return UMI_STUDIO_PANE_RUN_DEBUG;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "ai-assistant") == 0)
        return UMI_STUDIO_PANE_AI;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "ai-chat") == 0)
        return UMI_STUDIO_PANE_CHAT;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "ai-model-comparison") == 0)
        return UMI_STUDIO_PANE_AI_MODEL_COMPARISON;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "knowledge") == 0)
        return UMI_STUDIO_PANE_KNOWLEDGE;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "context-inspector") == 0)
        return UMI_STUDIO_PANE_AI_CONTEXT;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(panel->panel_id, "quality") == 0)
        return UMI_STUDIO_PANE_ARCHITECTURE;
    return NULL;
}

/*
 * Provide the studio workspace layout default operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_layout_default(
    UmiUiWorkspaceLayout *out_layout)
{
    return umi_application_suite_layout_project_default(
        "org.umicom.studio", out_layout);
}

/*
 * Provide the studio workspace layout select operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_layout_select(
    const char *layout_id,
    UmiUiWorkspaceLayout *out_layout)
{
    const UmiApplicationExperienceDefinition *experience;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (layout_id == NULL || out_layout == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    experience = umi_application_experience_catalogue_find("org.umicom.studio");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (experience == NULL) return UMI_STATUS_NOT_FOUND;
    return umi_application_suite_layout_project(experience, layout_id, out_layout);
}

/*
 * Provide the studio workspace layout register workbench operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_workspace_layout_register_workbench(
    UmiUiWorkbench *workbench,
    int activate_default)
{
    const UmiApplicationExperienceDefinition *experience;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    experience = umi_application_experience_catalogue_find("org.umicom.studio");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (experience == NULL) return UMI_STATUS_NOT_FOUND;
    return umi_application_suite_layout_register_workbench_profiles_resolved(
        experience,
        workbench,
        activate_default,
        studio_pane_resolver,
        NULL);
}

/*
 * Provide the studio workspace layouts seed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_layouts_seed(
    UmiStudioProfessionalWorkspace *workspace)
{
    UmiUiWorkspaceCustomisation *model =
        umi_studio_professional_workspace_model(workspace);
    UmiApplicationContextHub *context_hub = NULL;
    UmiDesktopRuntime *desktop = NULL;
    UmiUiWorkspaceLayout default_layout;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (model == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    /* Validate and materialise Studio's canonical experience layout before
     * populating the existing professional desktop customisation catalogue. */
    status = umi_studio_workspace_layout_default(&default_layout);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = umi_application_context_hub_create(&context_hub);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK)
        status = umi_desktop_runtime_create(context_hub, &desktop);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_desktop_runtime_seed(desktop);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK)
        status = umi_desktop_seed_workspace_customisation(
            model, umi_desktop_runtime_layouts(desktop),
            umi_desktop_runtime_monitors(desktop), "studio");
    umi_desktop_runtime_destroy(desktop);
    umi_application_context_hub_destroy(context_hub);
    return status;
}
