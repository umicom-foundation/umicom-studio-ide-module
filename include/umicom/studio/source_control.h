/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/source_control.h
 *
 * PURPOSE:
 *   Expose provider-neutral source-control status, history and Git operations to Studio.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_SOURCE_CONTROL_H
#define UMICOM_STUDIO_SOURCE_CONTROL_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio source control service data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioSourceControlService UmiStudioSourceControlService;

/**
 * Represent the studio source control snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioSourceControlSnapshot {
    char root[UMI_VCS_PATH_CAPACITY];
    char provider_id[UMI_VCS_ID_CAPACITY];
    char branch[UMI_VCS_NAME_CAPACITY];
    char upstream[UMI_VCS_NAME_CAPACITY];
    int available;
    int ahead;
    int behind;
    size_t changes;
    size_t staged;
    size_t commits;
    size_t conflicts;
    size_t branches;
    size_t remotes;
    size_t tags;
    size_t operations;
    uint64_t revision;
    uint64_t capabilities;
} UmiStudioSourceControlSnapshot;

/**
 * Initialise studio source control service from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_source_control_service_create(
    const char *root,
    UmiStudioSourceControlService **out_service
);
/**
 * Release or reset state held by studio source control service so the same storage can be
 * reused safely.
 */
void umi_studio_source_control_service_destroy(
    UmiStudioSourceControlService *service
);
/**
 * Provide the studio source control service refresh operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_source_control_service_refresh(
    UmiStudioSourceControlService *service,
    size_t history_limit
);
/**
 * Provide the studio source control service stage operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_source_control_service_stage(
    UmiStudioSourceControlService *service,
    const char *path
);
/**
 * Provide the studio source control service unstage operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_source_control_service_unstage(
    UmiStudioSourceControlService *service,
    const char *path
);
/**
 * Provide the studio source control service stage all operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_source_control_service_stage_all(UmiStudioSourceControlService *service);
/**
 * Provide the studio source control service unstage all operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_source_control_service_unstage_all(UmiStudioSourceControlService *service);
/**
 * Provide the studio source control service discard operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_source_control_service_discard(UmiStudioSourceControlService *service, const char *path);
/**
 * Provide the studio source control service commit operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_source_control_service_commit(
    UmiStudioSourceControlService *service,
    const char *message,
    char *out_commit_id,
    size_t capacity
);
/**
 * Provide the studio source control service fetch operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_source_control_service_fetch(UmiStudioSourceControlService *service);
/**
 * Provide the studio source control service pull operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_source_control_service_pull(UmiStudioSourceControlService *service);
/**
 * Provide the studio source control service push operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_source_control_service_push(UmiStudioSourceControlService *service);
/**
 * Initialise studio source control service branch from caller-provided values so later
 * operations receive a known state.
 */
UmiStatus umi_studio_source_control_service_branch_create(UmiStudioSourceControlService *service, const char *name, int checkout);
/**
 * Provide the studio source control service branch checkout operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_source_control_service_branch_checkout(UmiStudioSourceControlService *service, const char *name);
/**
 * Provide the studio source control service branch delete operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_source_control_service_branch_delete(UmiStudioSourceControlService *service, const char *name, int force);
/**
 * Provide the studio source control service open diff operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_source_control_service_open_diff(UmiStudioSourceControlService *service, const char *path, int staged);
/**
 * Provide the studio source control service snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_source_control_service_snapshot(
    const UmiStudioSourceControlService *service,
    UmiStudioSourceControlSnapshot *out_snapshot
);
/**
 * Provide the studio source control service changes operation used by this module and its
 * client applications.
 */
const UmiVcsChangeList *umi_studio_source_control_service_changes(
    const UmiStudioSourceControlService *service
);
/**
 * Provide the studio source control service history operation used by this module and its
 * client applications.
 */
const UmiVcsHistory *umi_studio_source_control_service_history(
    const UmiStudioSourceControlService *service
);
/**
 * Provide the studio source control service workspace operation used by this module and
 * its client applications.
 */
UmiVcsWorkspace *umi_studio_source_control_service_workspace(UmiStudioSourceControlService *service);
/**
 * Provide the studio source control service coordinator operation used by this module and
 * its client applications.
 */
UmiVcsWorkspaceCoordinator *umi_studio_source_control_service_coordinator(
    UmiStudioSourceControlService *service
);

#ifdef __cplusplus
}
#endif

#endif
