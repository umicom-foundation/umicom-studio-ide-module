/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/developer_workbench.h
 * PURPOSE: Aggregate the complete Studio developer platform over Framework services.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DEVELOPER_WORKBENCH_H
#define UMICOM_STUDIO_DEVELOPER_WORKBENCH_H
#include <stddef.h>
#include <stdint.h>
#include "umicom/studio/project_centre.h"
#include "umicom/studio/language_intelligence_centre.h"
#include "umicom/studio/debug_centre.h"
#include "umicom/studio/source_control_centre.h"
#include "umicom/studio/test_explorer_centre.h"
#include "umicom/studio/problems_centre.h"
#include "umicom/studio/output_centre.h"
#include "umicom/studio/progress_centre.h"
#include "umicom/studio/developer_session.h"
#include "umicom/studio/developer_pipeline_centre.h"
#include "umicom/studio/developer_task_centre.h"
#include "umicom/studio/developer_run_centre.h"
#include "umicom/studio/developer_workspace_state_centre.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio developer workbench data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDeveloperWorkbench UmiStudioDeveloperWorkbench;
/**
 * Represent the studio developer workbench snapshot data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioDeveloperWorkbenchSnapshot {uint32_t struct_size;uint32_t api_version;UmiStudioProjectCentreSnapshot projects;UmiStudioLanguageIntelligenceCentreSnapshot language;UmiStudioDebugCentreSnapshot debug;UmiStudioSourceControlCentreSnapshot source_control;UmiStudioTestExplorerCentreSnapshot tests;UmiStudioProblemsCentreSnapshot problems;UmiStudioOutputCentreSnapshot output;UmiStudioProgressCentreSnapshot progress;UmiStudioDeveloperSessionSnapshot session;uint64_t revision;size_t item_count;int available;UmiStudioDeveloperPipelineCentreSnapshot pipeline;UmiStudioDeveloperTaskCentreSnapshot tasks;UmiStudioDeveloperRunCentreSnapshot run;UmiStudioDeveloperWorkspaceStateCentreSnapshot workspace_state;} UmiStudioDeveloperWorkbenchSnapshot;
/**
 * Initialise studio developer workbench from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_developer_workbench_create(UmiStudioDeveloperWorkbench **out_workbench);
/**
 * Release or reset state held by studio developer workbench so the same storage can be
 * reused safely.
 */
void umi_studio_developer_workbench_destroy(UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_workbench_snapshot(UmiStudioDeveloperWorkbench *workbench,UmiStudioDeveloperWorkbenchSnapshot *out_snapshot);
/**
 * Provide the studio developer workbench projects operation used by this module and its
 * client applications.
 */
UmiStudioProjectCentre *umi_studio_developer_workbench_projects(UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench language operation used by this module and its
 * client applications.
 */
UmiStudioLanguageIntelligenceCentre *umi_studio_developer_workbench_language(UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench debug operation used by this module and its
 * client applications.
 */
UmiStudioDebugCentre *umi_studio_developer_workbench_debug(UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench source control operation used by this module and
 * its client applications.
 */
UmiStudioSourceControlCentre *umi_studio_developer_workbench_source_control(UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench tests operation used by this module and its
 * client applications.
 */
UmiStudioTestExplorerCentre *umi_studio_developer_workbench_tests(UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench services operation used by this module and its
 * client applications.
 */
UmiUiWorkbenchServices *umi_studio_developer_workbench_services(UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench runtime operation used by this module and its
 * client applications.
 */
UmiDeveloperRuntime *umi_studio_developer_workbench_runtime(UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench universal model operation used by this module and
 * its client applications.
 */
UmiDeveloperUniversalModel *umi_studio_developer_workbench_universal_model(
    UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench pipeline operation used by this module and its
 * client applications.
 */
UmiStudioDeveloperPipelineCentre *umi_studio_developer_workbench_pipeline(UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench task centre operation used by this module and its
 * client applications.
 */
UmiStudioDeveloperTaskCentre *umi_studio_developer_workbench_task_centre(UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench run centre operation used by this module and its
 * client applications.
 */
UmiStudioDeveloperRunCentre *umi_studio_developer_workbench_run_centre(UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench workspace state operation used by this module and
 * its client applications.
 */
UmiStudioDeveloperWorkspaceStateCentre *umi_studio_developer_workbench_workspace_state(UmiStudioDeveloperWorkbench *workbench);
/**
 * Provide the studio developer workbench session operation used by this module and its
 * client applications.
 */
UmiStudioDeveloperSession *umi_studio_developer_workbench_session(UmiStudioDeveloperWorkbench *workbench);

/**
 * Provide the studio developer workbench activate project operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_developer_workbench_activate_project(
    UmiStudioDeveloperWorkbench *workbench,
    const UmiProjectWorkspaceSelectionRequest *request,
    UmiProjectWorkspaceSelectionSnapshot *out_selection);

/**
 * Provide the studio developer workbench prepare project workflow operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_developer_workbench_prepare_project_workflow(
    UmiStudioDeveloperWorkbench *workbench,
    const UmiDeveloperProjectWorkflowRequest *request,
    UmiDeveloperProjectWorkflowSnapshot *out_workflow);

/**
 * Provide the studio developer workbench validate project operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_developer_workbench_validate_project(
    UmiStudioDeveloperWorkbench *workbench,
    UmiProjectWorkspaceValidationReport *out_report);

/**
 * Provide the studio developer workbench import project operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_developer_workbench_import_project(
    UmiStudioDeveloperWorkbench *workbench,
    const UmiDeveloperProjectBootstrapRequest *request,
    UmiDeveloperProjectBootstrapSnapshot *out_snapshot);

#ifdef __cplusplus
}
#endif
#endif
