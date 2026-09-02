/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/test_explorer_centre.h
 *
 * PURPOSE:
 *   Compose the Studio Test Explorer Centre over Framework-owned discovery,
 *   execution, rerun, coverage, benchmark and history services. Studio owns
 *   placement and product wiring only.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio owns product composition only. The reusable data model and service
 * remain in Umicom Framework so future applications can consume the same API.
 */
#ifndef UMICOM_STUDIO_TEST_EXPLORER_CENTRE_H
#define UMICOM_STUDIO_TEST_EXPLORER_CENTRE_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/test_platform/command.h"
#include "umicom/test_platform/explorer_session.h"
#include "umicom/studio/tests.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_TEST_EXPLORER_CENTRE_API_VERSION 4U

/**
 * Represent the studio test explorer centre data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioTestExplorerCentre UmiStudioTestExplorerCentre;

/**
 * Represent the studio test explorer command contribution data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioTestExplorerCommandContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu_id;
    const char *menu_group;
    int32_t order;
    int show_in_command_centre;
} UmiStudioTestExplorerCommandContribution;

/**
 * Represent the studio test explorer view contribution data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioTestExplorerViewContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *view_id;
    const char *title;
    const char *framework_contract;
    const char *default_region;
    int32_t order;
    int closable;
    int movable;
} UmiStudioTestExplorerViewContribution;

/**
 * Represent the studio test explorer centre snapshot data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioTestExplorerCentreSnapshot {
    uint32_t struct_size;
    uint32_t api_version;
    char area_id[128];
    char title[256];
    char summary[512];
    UmiTestPlatformServiceSnapshot service;
    UmiTestWorkspaceSnapshot workspace;
    UmiTestExplorerSessionSnapshot experience;
    UmiStudioTestExplorerState explorer;
    size_t hierarchy_count;
    size_t selected_count;
    size_t command_contribution_count;
    size_t view_contribution_count;
    uint64_t revision;
    int operation_running;
    int available;
} UmiStudioTestExplorerCentreSnapshot;

/**
 * Initialise studio test explorer centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_test_explorer_centre_create(
    UmiStudioTestExplorerCentre **out_centre);
/**
 * Provide the studio test explorer centre create bound operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_test_explorer_centre_create_bound(
    UmiStudioTestService *test_service,
    UmiStudioTestExplorerCentre **out_centre);
/**
 * Release or reset state held by studio test explorer centre so the same storage can be
 * reused safely.
 */
void umi_studio_test_explorer_centre_destroy(
    UmiStudioTestExplorerCentre *centre);
/**
 * Provide the studio test explorer centre snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_explorer_centre_snapshot(
    UmiStudioTestExplorerCentre *centre,
    UmiStudioTestExplorerCentreSnapshot *out_snapshot);
/**
 * Provide the studio test explorer centre service operation used by this module and its
 * client applications.
 */
UmiTestPlatformService *umi_studio_test_explorer_centre_service(
    UmiStudioTestExplorerCentre *centre);
/**
 * Provide the studio test explorer centre workspace operation used by this module and its
 * client applications.
 */
UmiTestWorkspace *umi_studio_test_explorer_centre_workspace(
    UmiStudioTestExplorerCentre *centre);
/**
 * Provide the studio test explorer centre experience operation used by this module and its
 * client applications.
 */
UmiTestExplorerSession *umi_studio_test_explorer_centre_experience(
    UmiStudioTestExplorerCentre *centre);
/**
 * Provide the studio test explorer centre set workspace operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_test_explorer_centre_set_workspace(
    UmiStudioTestExplorerCentre *centre,
    const char *workspace_root,
    const char *project_id,
    uint64_t workspace_revision);
/**
 * Provide the studio test explorer centre set filter operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_explorer_centre_set_filter(
    UmiStudioTestExplorerCentre *centre,
    const char *search_text,
    const char *label,
    int outcome,
    int include_disabled);
/**
 * Provide the studio test explorer centre hierarchy operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_explorer_centre_hierarchy(
    UmiStudioTestExplorerCentre *centre,
    UmiTestPlatformHierarchyNode *nodes,
    size_t capacity,
    size_t *out_count);
/**
 * Provide the studio test explorer centre plan all operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_explorer_centre_plan_all(
    UmiStudioTestExplorerCentre *centre,
    uint32_t repeat_count,
    int stop_on_failure,
    UmiTestPlatformOperationPlan *out_plan);
/**
 * Provide the studio test explorer centre plan failed operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_test_explorer_centre_plan_failed(
    UmiStudioTestExplorerCentre *centre,
    UmiTestPlatformOperationPlan *out_plan);
/**
 * Provide the studio test explorer centre begin operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_explorer_centre_begin(
    UmiStudioTestExplorerCentre *centre,
    const UmiTestPlatformOperationPlan *plan);
/**
 * Provide the studio test explorer centre stop operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_explorer_centre_stop(
    UmiStudioTestExplorerCentre *centre);
/**
 * Provide the studio test explorer centre finish operation used by this module and its
 * client applications.
 */
void umi_studio_test_explorer_centre_finish(
    UmiStudioTestExplorerCentre *centre);

/**
 * Return the number of records represented by studio test explorer command contribution
 * without changing their state.
 */
size_t umi_studio_test_explorer_command_contribution_count(void);
/**
 * Find studio test explorer command contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioTestExplorerCommandContribution *
umi_studio_test_explorer_command_contribution_at(size_t position);
/**
 * Find studio test explorer command contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioTestExplorerCommandContribution *
umi_studio_test_explorer_command_contribution_find(
    const char *framework_command_id);
/**
 * Return the number of records represented by studio test explorer view contribution
 * without changing their state.
 */
size_t umi_studio_test_explorer_view_contribution_count(void);
/**
 * Find studio test explorer view contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioTestExplorerViewContribution *
umi_studio_test_explorer_view_contribution_at(size_t position);
/**
 * Find studio test explorer view contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioTestExplorerViewContribution *
umi_studio_test_explorer_view_contribution_find(const char *view_id);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_TEST_EXPLORER_CENTRE_H */
