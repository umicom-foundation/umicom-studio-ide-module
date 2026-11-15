/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/services_codeguard.inc
 *
 * PURPOSE:
 *   Coordinate workspace identity and lifetime around Framework CodeGuard reviews.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static UmiStatus StudioCodeGuardOwner(const UmiStudioServices *services)
{
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return services->codeguard_owner_thread == umi_thread_current_id()
        ? UMI_STATUS_OK : UMI_STATUS_INVALID_STATE;
}

static UmiCodeGuardReviewSnapshot StudioCodeGuardEmpty(void)
{
    UmiCodeGuardReviewSnapshot snapshot = {0};
    snapshot.state = UMI_TASK_CREATED;
    snapshot.status = UMI_STATUS_NOT_FOUND;
    snapshot.operation_status = UMI_STATUS_NOT_FOUND;
    return snapshot;
}

/* The service keeps one graph for its whole lifetime. Framework guarantees
 * non-reused revisions; root comparison additionally prevents accidental
 * cross-root use of the review's independently retained absolute directory. */
static uint64_t StudioCodeGuardContext(const UmiStudioServices *services,
    const UmiWorkspaceGraphSnapshot *workspace)
{
    return workspace->open && umi_path_equal(workspace->root, services->codeguard_root)
        ? workspace->revision : 0U;
}

UmiStatus UmiStudioCodeGuardSnapshot(const UmiStudioServices *services,
    UmiCodeGuardReviewSnapshot *out)
{
    if (out == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStatus status = StudioCodeGuardOwner(services);
    if (status != UMI_STATUS_OK) return status;
    UmiWorkspaceGraphSnapshot workspace;
    status = umi_workspace_graph_snapshot(services->workspace, &workspace);
    if (status != UMI_STATUS_OK) return status;
    UmiCodeGuardReviewSnapshot snapshot = services->codeguard_snapshot;
    uint64_t context = StudioCodeGuardContext(services, &workspace);
    if (context == 0U || context != snapshot.report_context_id) {
        snapshot.has_report = 0;
        memset(&snapshot.summary, 0, sizeof(snapshot.summary));
    }
    *out = snapshot;
    return UMI_STATUS_OK;
}

UmiStatus UmiStudioCodeGuardPoll(UmiStudioServices *services,
    UmiCodeGuardReviewSnapshot *out)
{
    if (out == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStatus status = StudioCodeGuardOwner(services);
    if (status != UMI_STATUS_OK) return status;
    UmiWorkspaceGraphSnapshot workspace;
    status = umi_workspace_graph_snapshot(services->workspace, &workspace);
    if (status != UMI_STATUS_OK) return status;
    UmiCodeGuardReviewSnapshot snapshot = StudioCodeGuardEmpty();
    if (services->codeguard_review != NULL) {
        status = UmiCodeGuardReviewPoll(services->codeguard_review,
            StudioCodeGuardContext(services, &workspace), &snapshot);
        if (status != UMI_STATUS_OK) return status;
    }
    services->codeguard_snapshot = snapshot;
    *out = snapshot;
    return UMI_STATUS_OK;
}

UmiStatus UmiStudioCodeGuardBegin(UmiStudioServices *services,
    const UmiCodeGuardProfile *profile, uint64_t *out_request_id)
{
    if (out_request_id == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStatus status = StudioCodeGuardOwner(services);
    if (status != UMI_STATUS_OK) return status;
    UmiWorkspaceGraphSnapshot workspace;
    status = umi_workspace_graph_snapshot(services->workspace, &workspace);
    if (status != UMI_STATUS_OK) return status;
    if (!workspace.open || workspace.revision == 0U) return UMI_STATUS_INVALID_STATE;
    UmiCodeGuardReviewSnapshot snapshot;
    status = UmiStudioCodeGuardPoll(services, &snapshot);
    if (status != UMI_STATUS_OK) return status;
    if (snapshot.pending) return UMI_STATUS_BUSY;

    if (services->codeguard_review == NULL ||
        !umi_path_equal(services->codeguard_root, workspace.root)) {
        UmiCodeGuardReview *candidate = NULL;
        status = UmiCodeGuardReviewCreate(workspace.root, &candidate);
        if (status != UMI_STATUS_OK) return status;
        /* No callback remains after Poll retires the old job. Prepare the new
         * controller before releasing its predecessor so allocation failure
         * preserves the previous owner. No scan starts during root rebinding. */
        status = UmiCodeGuardReviewDestroy(&services->codeguard_review);
        if (status != UMI_STATUS_OK) {
            (void)UmiCodeGuardReviewDestroy(&candidate);
            return status;
        }
        services->codeguard_review = candidate;
        memcpy(services->codeguard_root, workspace.root, strlen(workspace.root) + 1U);
        services->codeguard_snapshot = StudioCodeGuardEmpty();
    }
    uint64_t request_id;
    status = UmiCodeGuardReviewBegin(services->codeguard_review,
        services->task_queue, profile, workspace.revision, &request_id);
    if (status != UMI_STATUS_OK) return status;
    /* Admission is now complete. Do not add a fallible poll after scheduling:
     * a caller must never receive failure while an unreported job is running. */
    snapshot = services->codeguard_snapshot;
    snapshot.request_id = request_id;
    snapshot.context_id = workspace.revision;
    snapshot.state = UMI_TASK_QUEUED;
    snapshot.status = UMI_STATUS_BUSY;
    snapshot.operation_status = UMI_STATUS_BUSY;
    snapshot.pending = 1;
    snapshot.cancel_requested = 0;
    snapshot.stale = 0;
    services->codeguard_snapshot = snapshot;
    *out_request_id = request_id;
    return UMI_STATUS_OK;
}

UmiStatus UmiStudioCodeGuardCancel(UmiStudioServices *services)
{
    UmiStatus status = StudioCodeGuardOwner(services);
    if (status != UMI_STATUS_OK) return status;
    if (services->codeguard_review == NULL) return UMI_STATUS_OK;
    status = UmiCodeGuardReviewCancel(services->codeguard_review);
    if (status == UMI_STATUS_OK && services->codeguard_snapshot.pending)
        services->codeguard_snapshot.cancel_requested = 1;
    return status;
}

const UmiCodeGuardResult *UmiStudioCodeGuardResult(const UmiStudioServices *services)
{
    if (StudioCodeGuardOwner(services) != UMI_STATUS_OK ||
        services->codeguard_review == NULL) return NULL;
    UmiWorkspaceGraphSnapshot workspace;
    if (umi_workspace_graph_snapshot(services->workspace, &workspace) != UMI_STATUS_OK)
        return NULL;
    return UmiCodeGuardReviewResult(services->codeguard_review,
        StudioCodeGuardContext(services, &workspace));
}
