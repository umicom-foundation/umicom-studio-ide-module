/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/project_centre.h
 *
 * PURPOSE:
 *   Compose the Project Centre over reusable Umicom Framework services.
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
#ifndef UMICOM_STUDIO_PROJECT_CENTRE_H
#define UMICOM_STUDIO_PROJECT_CENTRE_H
#include <stdint.h>
#include "umicom/project/project.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio project centre data shared with callers of this public contract.
 */
typedef struct UmiStudioProjectCentre UmiStudioProjectCentre;
#define UMI_STUDIO_PROJECT_CENTRE_API_VERSION 2U
/**
 * Represent the studio project centre refresh summary data shared with callers of this
 * public contract.
 */
typedef struct UmiStudioProjectCentreRefreshSummary {
    uint64_t source_revision;
    size_t discovered_project_count;
    size_t existing_project_count;
    size_t unchanged_project_count;
    size_t import_candidate_count;
    size_t missing_project_count;
    int requires_confirmation;
} UmiStudioProjectCentreRefreshSummary;
/**
 * Represent the studio project centre snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioProjectCentreSnapshot {
    uint32_t struct_size; uint32_t api_version;
    char area_id[128]; char title[256]; char summary[512];
    UmiProjectWorkspaceSnapshot service; uint64_t revision; int available;
    UmiProjectWorkspaceSelectionSnapshot selection;
    UmiProjectWorkspaceValidationReport validation;
    int has_selection;
    UmiProjectWorkspaceModelSnapshot workspace_model;
    UmiProjectWorkspaceOrderSnapshot build_order;
    UmiStudioProjectCentreRefreshSummary refresh;
    UmiStatus build_order_status;
    int has_workspace_model;
    int has_build_order;
    int has_refresh_plan;
} UmiStudioProjectCentreSnapshot;
/**
 * Initialise studio project centre from caller-provided values so later operations receive
 * a known state.
 */
UmiStatus umi_studio_project_centre_create(UmiStudioProjectCentre **out_centre);
/**
 * Release or reset state held by studio project centre so the same storage can be reused
 * safely.
 */
void umi_studio_project_centre_destroy(UmiStudioProjectCentre *centre);
/**
 * Provide the studio project centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_project_centre_snapshot(UmiStudioProjectCentre *centre,UmiStudioProjectCentreSnapshot *out_snapshot);
/**
 * Provide the studio project centre service operation used by this module and its client
 * applications.
 */
UmiProjectWorkspace *umi_studio_project_centre_service(UmiStudioProjectCentre *centre);
/**
 * Provide the studio project centre workspace model operation used by this module and its
 * client applications.
 */
UmiProjectWorkspaceModel *umi_studio_project_centre_workspace_model(
    UmiStudioProjectCentre *centre);

/**
 * Provide the studio project centre upsert workspace root operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_project_centre_upsert_workspace_root(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceRootSnapshot *root);
/**
 * Provide the studio project centre upsert project group operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_project_centre_upsert_project_group(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceGroupSnapshot *group);
/**
 * Provide the studio project centre upsert project member operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_project_centre_upsert_project_member(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceMemberSnapshot *member);
/**
 * Provide the studio project centre upsert workspace setting operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_project_centre_upsert_workspace_setting(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceSettingSnapshot *setting);
/**
 * Provide the studio project centre upsert workspace exclusion operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_project_centre_upsert_workspace_exclusion(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceExclusionSnapshot *exclusion);
/**
 * Provide the studio project centre plan workspace refresh operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_project_centre_plan_workspace_refresh(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceDiscoveryOptions *options,
    UmiProjectWorkspaceRefreshSnapshot *out_refresh);

/**
 * Provide the studio project centre select operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_project_centre_select(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceSelectionRequest *request,
    UmiProjectWorkspaceSelectionSnapshot *out_selection);

/**
 * Provide the studio project centre select project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_project_centre_select_project(
    UmiStudioProjectCentre *centre,
    const char *project_id,
    UmiProjectWorkspaceSelectionSnapshot *out_selection);

/**
 * Provide the studio project centre current selection operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_project_centre_current_selection(
    UmiStudioProjectCentre *centre,
    UmiProjectWorkspaceSelectionSnapshot *out_selection);

/**
 * Check that studio project centre satisfies its contract before another service relies on
 * it.
 */
UmiStatus umi_studio_project_centre_validate(
    UmiStudioProjectCentre *centre,
    UmiProjectWorkspaceValidationReport *out_report);

/**
 * Provide the studio project centre import directory operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_project_centre_import_directory(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceImportRequest *request,
    UmiProjectWorkspaceImportSnapshot *out_snapshot);

#ifdef __cplusplus
}
#endif
#endif
