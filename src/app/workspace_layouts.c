/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/workspace_layouts.c
 *
 * PURPOSE:
 *   Adapt the Framework-owned professional desktop layout catalogue to the
 *   Studio semantic workspace model. Studio contains no private layout logic;
 *   every definition and reusable behaviour comes from Umicom Framework.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/workspace_layouts.h"

#include <string.h>

#include "umicom/application/experience_catalogue.h"
#include "umicom/desktop/desktop.h"
#include "umicom/studio/contributions.h"

static const char *studio_pane_resolver(
    const UmiExperiencePanelDefinition *panel,
    void *user_data)
{
    (void)user_data;
    if (panel == NULL || panel->panel_id == NULL) return NULL;
    if (strcmp(panel->panel_id, "resource-explorer") == 0)
        return UMI_STUDIO_PANE_EXPLORER;
    if (strcmp(panel->panel_id, "editor") == 0)
        return UMI_STUDIO_PANE_EDITOR;
    if (strcmp(panel->panel_id, "problems") == 0)
        return UMI_STUDIO_PANE_PROBLEMS;
    if (strcmp(panel->panel_id, "output") == 0)
        return UMI_STUDIO_PANE_OUTPUT;
    if (strcmp(panel->panel_id, "terminal") == 0)
        return UMI_STUDIO_PANE_TERMINAL;
    if (strcmp(panel->panel_id, "source-control") == 0)
        return UMI_STUDIO_PANE_SOURCE_CONTROL;
    if (strcmp(panel->panel_id, "test-explorer") == 0)
        return UMI_STUDIO_PANE_TESTING;
    if (strcmp(panel->panel_id, "debug") == 0)
        return UMI_STUDIO_PANE_RUN_DEBUG;
    if (strcmp(panel->panel_id, "ai-assistant") == 0)
        return UMI_STUDIO_PANE_AI;
    if (strcmp(panel->panel_id, "knowledge") == 0)
        return UMI_STUDIO_PANE_KNOWLEDGE;
    if (strcmp(panel->panel_id, "context-inspector") == 0)
        return UMI_STUDIO_PANE_AI_CONTEXT;
    if (strcmp(panel->panel_id, "quality") == 0)
        return UMI_STUDIO_PANE_ARCHITECTURE;
    return NULL;
}

UmiStatus umi_studio_workspace_layout_default(
    UmiUiWorkspaceLayout *out_layout)
{
    return umi_application_suite_layout_project_default(
        "org.umicom.studio", out_layout);
}

UmiStatus umi_studio_workspace_layout_select(
    const char *layout_id,
    UmiUiWorkspaceLayout *out_layout)
{
    const UmiApplicationExperienceDefinition *experience;
    if (layout_id == NULL || out_layout == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    experience = umi_application_experience_catalogue_find("org.umicom.studio");
    if (experience == NULL) return UMI_STATUS_NOT_FOUND;
    return umi_application_suite_layout_project(experience, layout_id, out_layout);
}

UmiStatus umi_studio_workspace_layout_register_workbench(
    UmiUiWorkbench *workbench,
    int activate_default)
{
    const UmiApplicationExperienceDefinition *experience;
    if (workbench == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    experience = umi_application_experience_catalogue_find("org.umicom.studio");
    if (experience == NULL) return UMI_STATUS_NOT_FOUND;
    return umi_application_suite_layout_register_workbench_profiles_resolved(
        experience,
        workbench,
        activate_default,
        studio_pane_resolver,
        NULL);
}

UmiStatus umi_studio_workspace_layouts_seed(
    UmiStudioProfessionalWorkspace *workspace)
{
    UmiUiWorkspaceCustomisation *model =
        umi_studio_professional_workspace_model(workspace);
    UmiApplicationContextHub *context_hub = NULL;
    UmiDesktopRuntime *desktop = NULL;
    UmiUiWorkspaceLayout default_layout;
    UmiStatus status;
    if (model == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    /* Validate and materialise Studio's canonical experience layout before
     * populating the existing professional desktop customisation catalogue. */
    status = umi_studio_workspace_layout_default(&default_layout);
    if (status != UMI_STATUS_OK) return status;

    status = umi_application_context_hub_create(&context_hub);
    if (status == UMI_STATUS_OK)
        status = umi_desktop_runtime_create(context_hub, &desktop);
    if (status == UMI_STATUS_OK) status = umi_desktop_runtime_seed(desktop);
    if (status == UMI_STATUS_OK)
        status = umi_desktop_seed_workspace_customisation(
            model, umi_desktop_runtime_layouts(desktop),
            umi_desktop_runtime_monitors(desktop), "studio");
    umi_desktop_runtime_destroy(desktop);
    umi_application_context_hub_destroy(context_hub);
    return status;
}
