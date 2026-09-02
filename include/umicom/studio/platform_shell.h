/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/platform_shell.h
 *
 * PURPOSE:
 *   Define the Studio composition surface over reusable Framework workbench, resource, product, chart, designer and frontend platform services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio composes reusable Framework capabilities here. Generic models and
 * engines stay in Framework so the same mechanisms can serve future products.
 */
#ifndef APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_PLATFORM_SHELL_H
#define APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_PLATFORM_SHELL_H
#include <stddef.h>
#include <stdint.h>
#include "umicom/umicom.h"
#include "umicom/studio/services.h"
#include "umicom/studio/developer_workbench.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio platform shell data shared with callers of this public contract.
 */
typedef struct UmiStudioPlatformShell UmiStudioPlatformShell;
/**
 * Represent the studio platform shell snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioPlatformShellSnapshot {uint32_t struct_size;uint32_t api_version;char area_id[128];char view_type[128];char title[256];char summary[512];uint64_t revision;size_t item_count;int available;UmiUiWorkbenchPlatformSnapshot workbench;UmiResourceCentreSnapshot resources;UmiProductCentreSnapshot products;UmiChartWorkspaceSnapshot charts;UmiDesignerAuthoringSessionSnapshot designer;UmiFrontendApplicationSnapshot frontend;UmiStudioDeveloperWorkbenchSnapshot developer;} UmiStudioPlatformShellSnapshot;
/**
 * Initialise studio platform shell from caller-provided values so later operations receive
 * a known state.
 */
UmiStatus umi_studio_platform_shell_create(UmiStudioServices *services,UmiStudioPlatformShell **out_shell);
/**
 * Release or reset state held by studio platform shell so the same storage can be reused
 * safely.
 */
void umi_studio_platform_shell_destroy(UmiStudioPlatformShell *shell);
/**
 * Provide the studio platform shell snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_platform_shell_snapshot(UmiStudioPlatformShell *shell,UmiStudioPlatformShellSnapshot *out_snapshot);
/**
 * Provide the studio platform shell workbench operation used by this module and its client
 * applications.
 */
UmiUiWorkbenchPlatform *umi_studio_platform_shell_workbench(UmiStudioPlatformShell *shell);
/**
 * Provide the studio platform shell resources operation used by this module and its client
 * applications.
 */
UmiResourceCentre *umi_studio_platform_shell_resources(UmiStudioPlatformShell *shell);
/**
 * Provide the studio platform shell products operation used by this module and its client
 * applications.
 */
UmiProductCentre *umi_studio_platform_shell_products(UmiStudioPlatformShell *shell);
/**
 * Provide the studio platform shell charts operation used by this module and its client
 * applications.
 */
UmiChartWorkspace *umi_studio_platform_shell_charts(UmiStudioPlatformShell *shell);
/**
 * Provide the studio platform shell designer operation used by this module and its client
 * applications.
 */
UmiDesignerAuthoringSession *umi_studio_platform_shell_designer(UmiStudioPlatformShell *shell);
/**
 * Provide the studio platform shell frontend operation used by this module and its client
 * applications.
 */
UmiFrontendApplication *umi_studio_platform_shell_frontend(UmiStudioPlatformShell *shell);
/**
 * Provide the studio platform shell developer operation used by this module and its client
 * applications.
 */
UmiStudioDeveloperWorkbench *umi_studio_platform_shell_developer(UmiStudioPlatformShell *shell);

/**
 * Provide the studio platform shell activate project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_platform_shell_activate_project(
    UmiStudioPlatformShell *shell,
    const UmiProjectWorkspaceSelectionRequest *request,
    UmiProjectWorkspaceSelectionSnapshot *out_selection);

/**
 * Provide the studio platform shell validate project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_platform_shell_validate_project(
    UmiStudioPlatformShell *shell,
    UmiProjectWorkspaceValidationReport *out_report);

/**
 * Provide the studio platform shell prepare project workflow operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_platform_shell_prepare_project_workflow(
    UmiStudioPlatformShell *shell,
    const UmiDeveloperProjectWorkflowRequest *request,
    UmiDeveloperProjectWorkflowSnapshot *out_workflow);

/**
 * Provide the studio platform shell import project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_platform_shell_import_project(
    UmiStudioPlatformShell *shell,
    const UmiDeveloperProjectBootstrapRequest *request,
    UmiDeveloperProjectBootstrapSnapshot *out_snapshot);
#ifdef __cplusplus
}
#endif
#endif
