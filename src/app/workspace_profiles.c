/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/workspace_profiles.c
 *
 * PURPOSE:
 *   Register professional named workspace profiles with the live Framework
 *   workbench used by GTK4, headless tests and future frontend adapters.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* These profiles change only visible workbench chrome. They do not delete,
 * move or rewrite project files. Users can switch profiles from the toolbar,
 * the Window menu or the shared command registry, and the active selection is
 * saved in the normal Studio session state.
 */

#include "umicom/studio/workspace_profiles.h"

#include <stdio.h>
#include <string.h>

#include "umicom/application/experience_catalogue.h"
#include "umicom/studio/contributions.h"
#include "umicom/studio/workspace_layouts.h"

/*
 * Provide the is debug workspace pane operation used by this module and its client
 * applications.
 */
static int is_debug_workspace_pane(const char *pane_id)
{
    return strcmp(pane_id, UMI_STUDIO_PANE_RUN_DEBUG) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_DEBUG_CALL_STACK) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_DEBUG_VARIABLES) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_DEBUG_WATCHES) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_DEBUG_BREAKPOINTS) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_DEBUG_CONSOLE) == 0;
}

/*
 * Provide the is source control workspace pane operation used by this module and its
 * client applications.
 */
static int is_source_control_workspace_pane(const char *pane_id)
{
    return strcmp(pane_id, UMI_STUDIO_PANE_SOURCE_CONTROL) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_VCS_COMMIT) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_VCS_HISTORY) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_VCS_BRANCHES) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_VCS_REMOTES) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_VCS_CONFLICTS) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_VCS_DIFF) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_VCS_OPERATIONS) == 0;
}

/*
 * Provide the is testing workspace pane operation used by this module and its client
 * applications.
 */
static int is_testing_workspace_pane(const char *pane_id)
{
    return strcmp(pane_id, UMI_STUDIO_PANE_TESTING) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TEST_RESULTS) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TEST_FAILURES) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TEST_OUTPUT) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TEST_COVERAGE) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TEST_RUNS) == 0;
}

/*
 * Provide the is build workspace pane operation used by this module and its client
 * applications.
 */
static int is_build_workspace_pane(const char *pane_id)
{
    return strcmp(pane_id, UMI_STUDIO_PANE_BUILD_DASHBOARD) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_BUILD_GRAPH) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_BUILD_HISTORY) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_BUILD_OUTPUT) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_BUILD_ARTIFACTS) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_BUILD_TASKS) == 0;
}

/*
 * Provide the is trading workspace pane operation used by this module and its client
 * applications.
 */
static int is_trading_workspace_pane(const char *pane_id)
{
    return strcmp(pane_id, UMI_STUDIO_PANE_TRADING_DASHBOARD) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TRADING_WATCHLIST) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TRADING_DEPTH) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TRADING_CHART) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TRADING_ORDER_TICKET) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TRADING_ORDERS) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TRADING_EXECUTIONS) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TRADING_TIME_AND_SALES) == 0 ||
           strcmp(pane_id, UMI_STUDIO_PANE_TRADING_PORTFOLIO_RISK) == 0;
}

/* Combine Studio's always-available presentation profiles with the layouts in
 * the live Framework experience catalogue used by this installation. */
size_t umi_studio_workspace_profile_count(void)
{
    const UmiApplicationExperienceDefinition *experience =
        umi_application_experience_catalogue_find("org.umicom.studio");

    /* Built-in presentation profiles remain available even if the canonical
     * experience is unavailable during a damaged or partial installation. */
    return UMI_STUDIO_BUILTIN_WORKSPACE_PROFILE_COUNT +
        (experience != NULL ? experience->layout_count : 0U);
}

/* Provide the register profile operation used by this module and its client applications. */
static UmiStatus register_profile(UmiUiWorkbench *workbench,
                                  const char *profile_id,
                                  const char *label,
                                  const char *description,
                                  const char *icon_name,
                                  int sidebar_visible,
                                  int auxiliary_visible,
                                  int bottom_visible,
                                  int32_t sidebar_size,
                                  int32_t auxiliary_size,
                                  int32_t bottom_size,
                                  int32_t order)
{
    UmiUiWorkspaceProfileSnapshot profile;
    UmiUiWorkspaceProfileModel *model;
    size_t pane_index;
    (void)memset(&profile, 0, sizeof(profile));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    model = umi_ui_workbench_workspace_profiles(workbench);
    (void)snprintf(profile.profile_id, sizeof(profile.profile_id), "%s",
                   profile_id);
    (void)snprintf(profile.label, sizeof(profile.label), "%s", label);
    (void)snprintf(profile.description, sizeof(profile.description), "%s",
                   description);
    (void)snprintf(profile.icon_name, sizeof(profile.icon_name), "%s",
                   icon_name);
    profile.sidebar_visible = sidebar_visible;
    profile.auxiliary_sidebar_visible = auxiliary_visible;
    profile.bottom_panel_visible = bottom_visible;
    profile.sidebar_size = sidebar_size;
    profile.auxiliary_sidebar_size = auxiliary_size;
    profile.bottom_panel_size = bottom_size;
    profile.order = order;
    profile.built_in = 1;
    profile.locked = 1;

    /* Built-in layouts remember Studio's original tool-tab positions. This
     * makes Reset useful after a drag operation while still preserving every
     * pane and source file. Optional panes that are added later remain
     * untouched because Framework ignores unknown IDs during activation. */
    for (pane_index = 0U;
         pane_index < umi_ui_pane_model_count(
             umi_ui_workbench_panes(workbench));
         ++pane_index) {
        UmiUiPaneSnapshot pane;
        UmiUiWorkspacePanePlacement *saved;
        UmiStatus status = umi_ui_pane_model_at(
            umi_ui_workbench_panes(workbench), pane_index, &pane);
        int debug_pane;
        int source_control_pane;
        int testing_pane;
        int build_pane;
        int trading_pane;
        int profile_pane;
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        debug_pane = strcmp(profile_id,
                            UMI_STUDIO_WORKSPACE_PROFILE_DEBUG) == 0 &&
                     is_debug_workspace_pane(pane.pane_id);
        source_control_pane =
            strcmp(profile_id,
                   UMI_STUDIO_WORKSPACE_PROFILE_SOURCE_CONTROL) == 0 &&
            is_source_control_workspace_pane(pane.pane_id);
        testing_pane =
            strcmp(profile_id, UMI_STUDIO_WORKSPACE_PROFILE_TESTING) == 0 &&
            is_testing_workspace_pane(pane.pane_id);
        build_pane =
            strcmp(profile_id, UMI_STUDIO_WORKSPACE_PROFILE_BUILD) == 0 &&
            is_build_workspace_pane(pane.pane_id);
        trading_pane =
            strcmp(profile_id, UMI_STUDIO_WORKSPACE_PROFILE_TRADING) == 0 &&
            is_trading_workspace_pane(pane.pane_id);
        profile_pane = debug_pane || source_control_pane || testing_pane ||
                       build_pane || trading_pane;
        /* Apply this operation only while the related capability or state is available. */
        if ((!pane.visible && !profile_pane) ||
            pane.placement == UMI_UI_PLACEMENT_CENTRE ||
            pane.placement == UMI_UI_PLACEMENT_FLOATING) {
            continue;
        }
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (profile.pane_count >= UMI_UI_WORKSPACE_PROFILE_MAX_PANES) {
            return UMI_STATUS_CAPACITY_EXCEEDED;
        }
        saved = &profile.panes[profile.pane_count++];
        (void)snprintf(saved->pane_id, sizeof(saved->pane_id), "%s",
                       pane.pane_id);
        saved->placement = pane.placement;
        saved->order = pane.order;
        saved->visible = profile_pane ? 1 : pane.visible;
    }
    return umi_ui_workspace_profile_model_upsert(model, &profile);
}

/*
 * Add studio workspace profiles only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_workspace_profiles_register(UmiUiWorkbench *workbench)
{
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    status = register_profile(
        workbench, UMI_STUDIO_WORKSPACE_PROFILE_DEVELOP, "Develop",
        "Editor-first coding with project tools and a compact bottom panel",
        "applications-development-symbolic", 1, 0, 1, 288, 360, 240, 10);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = register_profile(
        workbench, UMI_STUDIO_WORKSPACE_PROFILE_FOCUS, "Focus",
        "Distraction-free editor with all tool regions temporarily hidden",
        "view-fullscreen-symbolic", 0, 0, 0, 288, 360, 240, 20);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = register_profile(
        workbench, UMI_STUDIO_WORKSPACE_PROFILE_DEBUG, "Debug",
        "Source, variables, call stack and an expanded debugging console",
        "applications-engineering-symbolic", 1, 1, 1, 300, 380, 300, 30);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = register_profile(
        workbench, UMI_STUDIO_WORKSPACE_PROFILE_SOURCE_CONTROL,
        "Source Control",
        "Changes, commit composition, history, branches, remotes, conflicts, diffs and operations",
        "org.gnome.Builder-vcs-symbolic", 1, 1, 1, 340, 420, 320, 40);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = register_profile(
        workbench, UMI_STUDIO_WORKSPACE_PROFILE_TESTING, "Testing",
        "Test Explorer, results, failures, output, coverage and run history",
        "emblem-ok-symbolic", 1, 1, 1, 340, 420, 320, 45);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = register_profile(
        workbench, UMI_STUDIO_WORKSPACE_PROFILE_BUILD, "Build",
        "Build dashboard, dependency graph, history, output, artifacts and tasks",
        "system-run-symbolic", 1, 1, 1, 360, 440, 340, 47);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = register_profile(
        workbench, UMI_STUDIO_WORKSPACE_PROFILE_REVIEW, "Review",
        "Code review and comparison with navigation and auxiliary context",
        "document-properties-symbolic", 1, 1, 1, 260, 420, 260, 50);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = register_profile(
        workbench, UMI_STUDIO_WORKSPACE_PROFILE_OPERATIONS, "Operations",
        "Monitoring, diagnostics, tasks and operational evidence",
        "utilities-system-monitor-symbolic", 1, 1, 1, 280, 420, 320, 60);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = register_profile(
        workbench, UMI_STUDIO_WORKSPACE_PROFILE_TRADING, "Trading",
        "Professional market, chart, order, risk and activity workspace",
        "view-statistics-symbolic", 1, 1, 1, 260, 390, 300, 70);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    /* Add the canonical Application Suite profiles to the same live workbench.
     * The resolver maps Framework experience panel IDs to Studio's existing
     * pane identities; no new Studio layout engine is introduced. */
    status = umi_studio_workspace_layout_register_workbench(workbench, 0);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    return umi_ui_workbench_activate_workspace_profile(
        workbench, UMI_STUDIO_WORKSPACE_PROFILE_DEVELOP);
}
