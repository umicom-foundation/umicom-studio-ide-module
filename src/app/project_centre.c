/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/project_centre.c
 *
 * PURPOSE:
 *   Implement the Project Centre composition.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/project_centre.h"
#include "umicom/platform/path.h"
#include <stdlib.h>
#include <string.h>

struct UmiStudioProjectCentre {
    UmiProjectWorkspace *service;
    UmiProjectWorkspaceModel *workspace_model;
    UmiProjectWorkspaceSelectionSnapshot selection;
    UmiProjectWorkspaceValidationReport validation;
    UmiProjectWorkspaceRefreshSnapshot refresh_plan;
    int has_selection;
    int has_refresh_plan;
    uint64_t revision;
};
/* Provide the copy text operation used by this module and its client applications. */
static void copy_text(char*d,size_t c,const char*s){size_t n;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(d==NULL||c==0U)return;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)s="";n=strlen(s);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(n>=c)n=c-1U;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(n>0U)memcpy(d,s,n);d[n]='\0';}

/*
 * Initialise studio project centre from caller-provided values so later operations receive
 * a known state.
 */
UmiStatus umi_studio_project_centre_create(UmiStudioProjectCentre **out)
{
    UmiStudioProjectCentre *centre;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out = NULL;
    centre = (UmiStudioProjectCentre *)calloc(1U, sizeof(*centre));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_project_workspace_create(&centre->service);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK)
        status = umi_project_workspace_model_create(
            centre->service, &centre->workspace_model);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_project_workspace_destroy(centre->service);
        free(centre);
        return status;
    }
    centre->revision = 1U;
    *out = centre;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio project centre so the same storage can be reused
 * safely.
 */
void umi_studio_project_centre_destroy(UmiStudioProjectCentre *centre)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return;
    umi_project_workspace_model_destroy(centre->workspace_model);
    umi_project_workspace_destroy(centre->service);
    free(centre);
}

/*
 * Provide the studio project centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_project_centre_snapshot(
    UmiStudioProjectCentre *centre,
    UmiStudioProjectCentreSnapshot *out_snapshot)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || out_snapshot == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->struct_size = (uint32_t)sizeof(*out_snapshot);
    out_snapshot->api_version = UMI_STUDIO_PROJECT_CENTRE_API_VERSION;
    copy_text(out_snapshot->area_id, sizeof(out_snapshot->area_id),
              "studio.project-centre");
    copy_text(out_snapshot->title, sizeof(out_snapshot->title),
              "Project Centre");
    copy_text(out_snapshot->summary, sizeof(out_snapshot->summary),
              "Multi-root projects, inherited settings, CMake discovery, "
              "tasks, launch profiles and deterministic build order.");
    status = umi_project_workspace_snapshot(
        centre->service, &out_snapshot->service);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_project_workspace_model_snapshot(
        centre->workspace_model, &out_snapshot->workspace_model);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    out_snapshot->has_workspace_model = 1;
    out_snapshot->build_order_status =
        umi_project_workspace_model_resolve_build_order(
            centre->workspace_model, &out_snapshot->build_order);
    out_snapshot->has_build_order =
        out_snapshot->build_order_status == UMI_STATUS_OK;
    status = umi_project_workspace_validate(
        centre->service, &centre->validation);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    out_snapshot->validation = centre->validation;
    /* Apply this branch only when its contract condition is satisfied. */
    if (centre->has_selection) {
        out_snapshot->selection = centre->selection;
        out_snapshot->has_selection = 1;
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (centre->has_refresh_plan) {
        out_snapshot->refresh.source_revision =
            centre->refresh_plan.source_revision;
        out_snapshot->refresh.discovered_project_count =
            centre->refresh_plan.discovery.project_count;
        out_snapshot->refresh.existing_project_count =
            centre->refresh_plan.existing_project_count;
        out_snapshot->refresh.unchanged_project_count =
            centre->refresh_plan.unchanged_project_count;
        out_snapshot->refresh.import_candidate_count =
            centre->refresh_plan.import_candidate_count;
        out_snapshot->refresh.missing_project_count =
            centre->refresh_plan.missing_project_count;
        out_snapshot->refresh.requires_confirmation =
            centre->refresh_plan.requires_confirmation;
        out_snapshot->has_refresh_plan = 1;
    }
    out_snapshot->revision = centre->revision +
                             out_snapshot->workspace_model.revision;
    out_snapshot->available = 1;
    return UMI_STATUS_OK;
}
/*
 * Provide the studio project centre service operation used by this module and its client
 * applications.
 */
UmiProjectWorkspace *umi_studio_project_centre_service(UmiStudioProjectCentre*p){return p!=NULL?p->service:NULL;}

/*
 * Provide the studio project centre workspace model operation used by this module and its
 * client applications.
 */
UmiProjectWorkspaceModel *umi_studio_project_centre_workspace_model(
    UmiStudioProjectCentre *centre)
{
    return centre != NULL ? centre->workspace_model : NULL;
}

/*
 * Provide the studio project centre upsert workspace root operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_project_centre_upsert_workspace_root(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceRootSnapshot *root)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_project_workspace_model_upsert_root(
        centre->workspace_model, root);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        centre->has_refresh_plan = 0;
        centre->revision += 1U;
    }
    return status;
}

/*
 * Provide the studio project centre upsert project group operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_project_centre_upsert_project_group(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceGroupSnapshot *group)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_project_workspace_model_upsert_group(
        centre->workspace_model, group);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        centre->has_refresh_plan = 0;
        centre->revision += 1U;
    }
    return status;
}

/*
 * Provide the studio project centre upsert project member operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_project_centre_upsert_project_member(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceMemberSnapshot *member)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_project_workspace_model_upsert_member(
        centre->workspace_model, member);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        centre->has_refresh_plan = 0;
        centre->revision += 1U;
    }
    return status;
}

/*
 * Provide the studio project centre upsert workspace setting operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_project_centre_upsert_workspace_setting(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceSettingSnapshot *setting)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_project_workspace_model_upsert_setting(
        centre->workspace_model, setting);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        centre->has_refresh_plan = 0;
        centre->revision += 1U;
    }
    return status;
}

/*
 * Provide the studio project centre upsert workspace exclusion operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_project_centre_upsert_workspace_exclusion(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceExclusionSnapshot *exclusion)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_project_workspace_model_upsert_exclusion(
        centre->workspace_model, exclusion);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        centre->has_refresh_plan = 0;
        centre->revision += 1U;
    }
    return status;
}

/*
 * Provide the studio project centre plan workspace refresh operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_project_centre_plan_workspace_refresh(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceDiscoveryOptions *options,
    UmiProjectWorkspaceRefreshSnapshot *out_refresh)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_project_workspace_model_plan_refresh(
        centre->workspace_model, options, &centre->refresh_plan);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    centre->has_refresh_plan = 1;
    centre->revision += 1U;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_refresh != NULL) *out_refresh = centre->refresh_plan;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio project centre select operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_project_centre_select(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceSelectionRequest *request,
    UmiProjectWorkspaceSelectionSnapshot *out_selection)
{
    UmiProjectWorkspaceSelectionSnapshot selection;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    status = umi_project_workspace_resolve_selection(
        centre->service, request, &selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }

    centre->selection = selection;
    centre->has_selection = 1;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (request != NULL && request->project_id != NULL) {
        UmiProjectWorkspaceMemberSnapshot member;
        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_project_workspace_model_find_project_member(
                centre->workspace_model, request->project_id, &member) ==
            UMI_STATUS_OK)
            (void)umi_project_workspace_model_set_active_project(
                centre->workspace_model, request->project_id);
    }
    centre->revision += 1U;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_selection != NULL) {
        *out_selection = selection;
    }
    return UMI_STATUS_OK;
}

/*
 * Provide the studio project centre select project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_project_centre_select_project(
    UmiStudioProjectCentre *centre,
    const char *project_id,
    UmiProjectWorkspaceSelectionSnapshot *out_selection)
{
    UmiProjectWorkspaceSelectionRequest request;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || project_id == NULL || project_id[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    memset(&request, 0, sizeof(request));
    request.struct_size = (uint32_t)sizeof(request);
    request.api_version = UMI_PROJECT_WORKSPACE_QUERY_API_VERSION;
    request.project_id = project_id;
    return umi_studio_project_centre_select(centre, &request, out_selection);
}

/*
 * Provide the studio project centre current selection operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_project_centre_current_selection(
    UmiStudioProjectCentre *centre,
    UmiProjectWorkspaceSelectionSnapshot *out_selection)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || out_selection == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (!centre->has_selection) {
        return UMI_STATUS_NOT_FOUND;
    }
    *out_selection = centre->selection;
    return UMI_STATUS_OK;
}

/*
 * Check that studio project centre satisfies its contract before another service relies on
 * it.
 */
UmiStatus umi_studio_project_centre_validate(
    UmiStudioProjectCentre *centre,
    UmiProjectWorkspaceValidationReport *out_report)
{
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || out_report == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    status = umi_project_workspace_validate(centre->service, &centre->validation);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    *out_report = centre->validation;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio project centre import directory operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_project_centre_import_directory(
    UmiStudioProjectCentre *centre,
    const UmiProjectWorkspaceImportRequest *request,
    UmiProjectWorkspaceImportSnapshot *out_snapshot)
{
    UmiProjectWorkspaceRootSnapshot root;
    UmiProjectWorkspaceMemberSnapshot member;
    UmiStatus status;
    size_t root_index;
    int reused_root = 0;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || request == NULL || out_snapshot == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

#define imported (*out_snapshot)
    status = umi_project_workspace_import_directory(
        centre->service, request, &imported);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }

    /*
     * Import remains non-destructive.  The imported project directory becomes
     * a model root only when no root with the same stable identifier exists;
     * upsert then makes repeated imports idempotent.  Root/member identifiers
     * live in separate registries, so the project identifier is a safe and
     * deterministic identity for both records without string-prefix overflow.
     */
    memset(&root, 0, sizeof(root));
    /* Visit each bounded item once so every record receives the same rule. */
    for (root_index = 0U;
         root_index < umi_project_workspace_model_root_count(
             centre->workspace_model);
         ++root_index) {
        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_project_workspace_model_root_at(
                centre->workspace_model, root_index, &root) == UMI_STATUS_OK &&
            umi_path_equal(root.path, imported.root_directory)) {
            reused_root = 1;
            break;
        }
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (!reused_root) {
        memset(&root, 0, sizeof(root));
        root.struct_size = (uint32_t)sizeof(root);
        root.api_version = UMI_PROJECT_WORKSPACE_MODEL_API_VERSION;
        copy_text(root.id, sizeof(root.id), imported.project_id);
        copy_text(root.path, sizeof(root.path), imported.root_directory);
        copy_text(root.label, sizeof(root.label), imported.display_name);
        root.order = (int32_t)umi_project_workspace_model_root_count(
            centre->workspace_model);
        root.enabled = 1;
        status = umi_project_workspace_model_upsert_root(
            centre->workspace_model, &root);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }
    memset(&member, 0, sizeof(member));
    member.struct_size = (uint32_t)sizeof(member);
    member.api_version = UMI_PROJECT_WORKSPACE_MODEL_API_VERSION;
    copy_text(member.id, sizeof(member.id), imported.project_id);
    copy_text(member.root_id, sizeof(member.root_id), imported.project_id);
    copy_text(member.project_id, sizeof(member.project_id), imported.project_id);
    member.order = (int32_t)umi_project_workspace_model_member_count(
        centre->workspace_model);
    member.enabled = 1;
    status = umi_project_workspace_model_upsert_member(
        centre->workspace_model, &member);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_project_workspace_model_set_active_project(
        centre->workspace_model, imported.project_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    centre->selection = imported.selection;
    centre->validation = imported.validation;
    centre->has_selection = 1;
    centre->has_refresh_plan = 0;
    centre->revision += 1U;
#undef imported
    return UMI_STATUS_OK;
}
