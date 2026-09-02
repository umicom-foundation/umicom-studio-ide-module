/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/test_explorer_centre.c
 *
 * PURPOSE:
 *   Compose Studio's Test Explorer over the professional Framework session and
 *   contribute Framework commands and views to the Studio workbench.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/test_explorer_centre.h"

#include <stdlib.h>
#include <string.h>

struct UmiStudioTestExplorerCentre {
    UmiTestPlatformService *service;
    UmiTestWorkspace *workspace;
    UmiTestExplorerSession *experience;
    UmiStudioTestService *test_service;
    uint64_t revision;
    int owns_service;
    int owns_workspace;
};

/* Provide the copy text operation used by this module and its client applications. */
static void copy_text(char *destination, size_t capacity, const char *source)
{
    size_t length;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (destination == NULL || capacity == 0U) return;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (source == NULL) source = "";

    length = strlen(source);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length >= capacity) length = capacity - 1U;
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length > 0U) (void)memcpy(destination, source, length);
    destination[length] = '\0';
}

#define COMMAND(command, menu, group, position)                              \
    {                                                                         \
        (uint32_t)sizeof(UmiStudioTestExplorerCommandContribution),           \
        UMI_STUDIO_TEST_EXPLORER_CENTRE_API_VERSION,                          \
        command, menu, group, position, 1                                     \
    }

static const UmiStudioTestExplorerCommandContribution COMMANDS[] = {
    COMMAND("test.explorer.open", "View", "tests", 100),
    COMMAND("test.explorer.refresh", "View", "tests", 110),
    COMMAND("test.discover", "Test", "discovery", 100),
    COMMAND("test.run-all", "Test", "run", 200),
    COMMAND("test.run-selected", "Test", "run", 210),
    COMMAND("test.debug-selected", "Test", "run", 220),
    COMMAND("test.coverage-all", "Test", "coverage", 300),
    COMMAND("test.coverage-selected", "Test", "coverage", 310),
    COMMAND("test.rerun-failed", "Test", "rerun", 400),
    COMMAND("test.rerun-last", "Test", "rerun", 410),
    COMMAND("test.rerun-flaky", "Test", "rerun", 420),
    COMMAND("test.repeat-selected", "Test", "rerun", 430),
    COMMAND("test.stop", "Test", "run", 230),
    COMMAND("test.profile.select", "Test", "configuration", 500),
    COMMAND("test.filter.focus", "View", "test-filter", 200),
    COMMAND("test.filter.failed", "View", "test-filter", 210),
    COMMAND("test.filter.not-run", "View", "test-filter", 220),
    COMMAND("test.filter.clear", "View", "test-filter", 230),
    COMMAND("test.group.hierarchy", "View", "test-group", 300),
    COMMAND("test.group.suite", "View", "test-group", 310),
    COMMAND("test.group.framework", "View", "test-group", 320),
    COMMAND("test.group.outcome", "View", "test-group", 330),
    COMMAND("test.sort.name", "View", "test-sort", 400),
    COMMAND("test.sort.duration", "View", "test-sort", 410),
    COMMAND("test.sort.outcome", "View", "test-sort", 420),
    COMMAND("test.explorer.expand-all", "View", "test-tree", 500),
    COMMAND("test.explorer.collapse-all", "View", "test-tree", 510),
    COMMAND("test.failure.next", "Navigate", "test-failure", 600),
    COMMAND("test.failure.previous", "Navigate", "test-failure", 610),
    COMMAND("test.source.open", "Navigate", "test-source", 620),
    COMMAND("test.output.open", "View", "test-results", 600),
    COMMAND("test.coverage.open", "View", "test-results", 610),
    COMMAND("test.benchmarks.open", "View", "test-results", 620),
    COMMAND("test.history.open", "View", "test-results", 630),
    COMMAND("test.rerun-planner.open", "View", "test-results", 640),
    COMMAND("test.results.clear", "Test", "clear", 700),
    COMMAND("test.output.clear", "Test", "clear", 710),
    COMMAND("test.coverage.clear", "Test", "clear", 720),
    COMMAND("test.result.copy", "Edit", "test-result", 700),
    COMMAND("test.report.export", "File", "export", 700)
};

#undef COMMAND

#define VIEW(view, label, contract, region, position)                         \
    {                                                                          \
        (uint32_t)sizeof(UmiStudioTestExplorerViewContribution),               \
        UMI_STUDIO_TEST_EXPLORER_CENTRE_API_VERSION,                           \
        view, label, contract, region, position, 1, 1                          \
    }

static const UmiStudioTestExplorerViewContribution VIEWS[] = {
    VIEW("studio.test.explorer", "Test Explorer",
         "umicom.test.explorer-model", "secondary-sidebar", 100),
    VIEW("studio.test.results", "Test Results",
         "umicom.test.workspace", "bottom-panel", 110),
    VIEW("studio.test.output", "Test Output",
         "umicom.test.output", "bottom-panel", 120),
    VIEW("studio.test.coverage", "Coverage",
         "umicom.test.coverage-analysis", "bottom-panel", 130),
    VIEW("studio.test.coverage-details", "Coverage Details",
         "umicom.test.coverage-file-analysis", "secondary-sidebar", 140),
    VIEW("studio.test.benchmarks", "Benchmarks",
         "umicom.test.benchmark-analysis", "bottom-panel", 150),
    VIEW("studio.test.history", "Test History",
         "umicom.test.history-analysis", "secondary-sidebar", 160),
    VIEW("studio.test.rerun-planner", "Rerun Planner",
         "umicom.test.rerun-policy", "secondary-sidebar", 170)
};

#undef VIEW

/* Provide the create experience operation used by this module and its client applications. */
static UmiStatus create_experience(UmiStudioTestExplorerCentre *centre)
{
    return umi_test_explorer_session_create_bound(
        centre->service, centre->workspace, &centre->experience);
}

/*
 * Initialise studio test explorer centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_test_explorer_centre_create(
    UmiStudioTestExplorerCentre **out_centre)
{
    UmiStudioTestExplorerCentre *centre;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_centre = NULL;

    centre = (UmiStudioTestExplorerCentre *)calloc(1U, sizeof(*centre));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    status = umi_test_platform_service_create(&centre->service);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        centre->owns_service = 1;
        status = umi_test_workspace_create(centre->service, &centre->workspace);
    }

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        centre->owns_workspace = 1;
        status = create_experience(centre);
    }

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_test_explorer_centre_destroy(centre);
        return status;
    }

    centre->revision = 1U;
    *out_centre = centre;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio test explorer centre create bound operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_test_explorer_centre_create_bound(
    UmiStudioTestService *test_service,
    UmiStudioTestExplorerCentre **out_centre)
{
    UmiStudioTestExplorerCentre *centre;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (test_service == NULL || out_centre == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    *out_centre = NULL;

    centre = (UmiStudioTestExplorerCentre *)calloc(1U, sizeof(*centre));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    centre->test_service = test_service;
    centre->service = umi_studio_test_service_platform(test_service);
    centre->workspace = umi_studio_test_service_workspace(test_service);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre->service == NULL || centre->workspace == NULL) {
        free(centre);
        return UMI_STATUS_INVALID_STATE;
    }

    status = create_experience(centre);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        free(centre);
        return status;
    }

    centre->revision = 1U;
    *out_centre = centre;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio test explorer centre so the same storage can be
 * reused safely.
 */
void umi_studio_test_explorer_centre_destroy(
    UmiStudioTestExplorerCentre *centre)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return;

    umi_test_explorer_session_destroy(centre->experience);

    /* Apply this branch only when its contract condition is satisfied. */
    if (centre->owns_workspace) {
        umi_test_workspace_destroy(centre->workspace);
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (centre->owns_service) {
        umi_test_platform_service_destroy(centre->service);
    }

    free(centre);
}

/*
 * Provide the studio test explorer centre snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_explorer_centre_snapshot(
    UmiStudioTestExplorerCentre *centre,
    UmiStudioTestExplorerCentreSnapshot *out_snapshot)
{
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || out_snapshot == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->struct_size = (uint32_t)sizeof(*out_snapshot);
    out_snapshot->api_version =
        UMI_STUDIO_TEST_EXPLORER_CENTRE_API_VERSION;

    copy_text(out_snapshot->area_id,
              sizeof(out_snapshot->area_id),
              "studio.test-explorer-centre");
    copy_text(out_snapshot->title,
              sizeof(out_snapshot->title),
              "Test Explorer Centre");
    copy_text(
        out_snapshot->summary,
        sizeof(out_snapshot->summary),
        "Framework-owned discovery, filtering, execution, reruns, "
        "coverage gates, benchmark analysis and retained test history.");

    status = umi_test_platform_service_snapshot(
        centre->service,
        &out_snapshot->service);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_test_workspace_snapshot(
            centre->workspace,
            &out_snapshot->workspace);
    }

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_test_explorer_session_refresh(
            centre->experience,
            NULL);
    }

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_test_explorer_session_snapshot(
            centre->experience,
            &out_snapshot->experience);
    }

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre->test_service != NULL) {
        UmiTestPlatformHierarchyNode *nodes =
            (UmiTestPlatformHierarchyNode *)calloc(
                UMI_TEST_PLATFORM_SELECTION_CAPACITY,
                sizeof(*nodes));

        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (nodes == NULL) {
            return UMI_STATUS_OUT_OF_MEMORY;
        }

        status = umi_studio_test_service_explorer_state(
            centre->test_service,
            &out_snapshot->explorer);

        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            status = umi_studio_test_service_hierarchy(
                centre->test_service,
                nodes,
                UMI_TEST_PLATFORM_SELECTION_CAPACITY,
                &out_snapshot->hierarchy_count);
        }

        free(nodes);

        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) {
            return status;
        }
    }

    /*
     * The Framework service owns OPERATION selection. The Explorer model owns
     * interactive ROW selection. A run-all operation can therefore have one or
     * more planned items even when no row was manually clicked.
     *
     * Using the service snapshot here keeps Studio thin and fixes the previous
     * duplicate-state bug exposed by studio.test_explorer_operations.
     */
    out_snapshot->selected_count = out_snapshot->service.selected_count;

    out_snapshot->command_contribution_count =
        umi_studio_test_explorer_command_contribution_count();
    out_snapshot->view_contribution_count =
        umi_studio_test_explorer_view_contribution_count();
    out_snapshot->operation_running =
        out_snapshot->service.operation_running;
    out_snapshot->revision =
        centre->revision > out_snapshot->experience.revision
            ? centre->revision
            : out_snapshot->experience.revision;
    out_snapshot->available = 1;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio test explorer centre service operation used by this module and its
 * client applications.
 */
UmiTestPlatformService *umi_studio_test_explorer_centre_service(
    UmiStudioTestExplorerCentre *centre)
{
    return centre != NULL ? centre->service : NULL;
}

/*
 * Provide the studio test explorer centre workspace operation used by this module and its
 * client applications.
 */
UmiTestWorkspace *umi_studio_test_explorer_centre_workspace(
    UmiStudioTestExplorerCentre *centre)
{
    return centre != NULL ? centre->workspace : NULL;
}

/*
 * Provide the studio test explorer centre experience operation used by this module and its
 * client applications.
 */
UmiTestExplorerSession *umi_studio_test_explorer_centre_experience(
    UmiStudioTestExplorerCentre *centre)
{
    return centre != NULL ? centre->experience : NULL;
}

/*
 * Provide the studio test explorer centre set workspace operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_test_explorer_centre_set_workspace(
    UmiStudioTestExplorerCentre *centre,
    const char *workspace_root,
    const char *project_id,
    uint64_t workspace_revision)
{
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || centre->test_service == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }

    status = umi_studio_test_service_set_workspace(
        centre->test_service,
        workspace_root,
        project_id,
        workspace_revision);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) ++centre->revision;
    return status;
}

/*
 * Provide the studio test explorer centre set filter operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_explorer_centre_set_filter(
    UmiStudioTestExplorerCentre *centre,
    const char *search_text,
    const char *label,
    int outcome,
    int include_disabled)
{
    UmiStatus status;
    UmiTestPlatformFilter filter;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre->test_service != NULL) {
        status = umi_studio_test_service_set_filter(
            centre->test_service,
            search_text,
            label,
            outcome,
            include_disabled);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }

    umi_test_platform_filter_init(&filter);
    copy_text(filter.text, sizeof(filter.text), search_text);
    copy_text(filter.label, sizeof(filter.label), label);
    filter.outcome = outcome;
    filter.include_disabled = include_disabled;
    filter.failed_only =
        outcome == UMI_TEST_PLATFORM_OUTCOME_FAILED;

    status = umi_test_explorer_session_set_filter(
        centre->experience,
        &filter);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) ++centre->revision;
    return status;
}

/*
 * Provide the studio test explorer centre hierarchy operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_explorer_centre_hierarchy(
    UmiStudioTestExplorerCentre *centre,
    UmiTestPlatformHierarchyNode *nodes,
    size_t capacity,
    size_t *out_count)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || centre->test_service == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }

    return umi_studio_test_service_hierarchy(
        centre->test_service,
        nodes,
        capacity,
        out_count);
}

/*
 * Provide the studio test explorer centre plan all operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_explorer_centre_plan_all(
    UmiStudioTestExplorerCentre *centre,
    uint32_t repeat_count,
    int stop_on_failure,
    UmiTestPlatformOperationPlan *out_plan)
{
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || centre->test_service == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }

    status = umi_studio_test_service_plan_all(
        centre->test_service,
        repeat_count,
        stop_on_failure,
        out_plan);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) ++centre->revision;
    return status;
}

/*
 * Provide the studio test explorer centre plan failed operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_test_explorer_centre_plan_failed(
    UmiStudioTestExplorerCentre *centre,
    UmiTestPlatformOperationPlan *out_plan)
{
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || centre->test_service == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }

    status = umi_studio_test_service_plan_failed(
        centre->test_service,
        out_plan);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) ++centre->revision;
    return status;
}

/*
 * Provide the studio test explorer centre begin operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_explorer_centre_begin(
    UmiStudioTestExplorerCentre *centre,
    const UmiTestPlatformOperationPlan *plan)
{
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || centre->test_service == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }

    status = umi_studio_test_service_begin(
        centre->test_service,
        plan);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) ++centre->revision;
    return status;
}

/*
 * Provide the studio test explorer centre stop operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_test_explorer_centre_stop(
    UmiStudioTestExplorerCentre *centre)
{
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || centre->test_service == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }

    status = umi_studio_test_service_stop(
        centre->test_service);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) ++centre->revision;
    return status;
}

/*
 * Provide the studio test explorer centre finish operation used by this module and its
 * client applications.
 */
void umi_studio_test_explorer_centre_finish(
    UmiStudioTestExplorerCentre *centre)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || centre->test_service == NULL) return;

    umi_studio_test_service_finish(centre->test_service);
    ++centre->revision;
}

/*
 * Return the number of records represented by studio test explorer command contribution
 * without changing their state.
 */
size_t umi_studio_test_explorer_command_contribution_count(void)
{
    return sizeof(COMMANDS) / sizeof(COMMANDS[0]);
}

/*
 * Find studio test explorer command contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioTestExplorerCommandContribution *
umi_studio_test_explorer_command_contribution_at(size_t position)
{
    return position <
            umi_studio_test_explorer_command_contribution_count()
        ? &COMMANDS[position]
        : NULL;
}

/*
 * Find studio test explorer command contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioTestExplorerCommandContribution *
umi_studio_test_explorer_command_contribution_find(
    const char *framework_command_id)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (framework_command_id == NULL) return NULL;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < umi_studio_test_explorer_command_contribution_count();
         ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(COMMANDS[index].framework_command_id,
                   framework_command_id) == 0) {
            return &COMMANDS[index];
        }
    }

    return NULL;
}

/*
 * Return the number of records represented by studio test explorer view contribution
 * without changing their state.
 */
size_t umi_studio_test_explorer_view_contribution_count(void)
{
    return sizeof(VIEWS) / sizeof(VIEWS[0]);
}

/*
 * Find studio test explorer view contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioTestExplorerViewContribution *
umi_studio_test_explorer_view_contribution_at(size_t position)
{
    return position <
            umi_studio_test_explorer_view_contribution_count()
        ? &VIEWS[position]
        : NULL;
}

/*
 * Find studio test explorer view contribution while leaving the underlying catalogue or
 * model owned by this module.
 */
const UmiStudioTestExplorerViewContribution *
umi_studio_test_explorer_view_contribution_find(const char *view_id)
{
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (view_id == NULL) return NULL;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < umi_studio_test_explorer_view_contribution_count();
         ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(VIEWS[index].view_id, view_id) == 0) {
            return &VIEWS[index];
        }
    }

    return NULL;
}
