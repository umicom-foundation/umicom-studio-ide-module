/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/context_link_centre.c
 *
 * PURPOSE:
 *   Create Studio's development, testing, AI and data context groups while delegating routing, history, pinning and delivery to Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "context_link_centre.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "umicom/workbench_context_host/ai_publisher.h"
#include "umicom/workbench_context_host/diagnostic_publisher.h"
#include "umicom/workbench_context_host/project_publisher.h"
#include "umicom/workbench_context_host/source_location_publisher.h"
#include "umicom/workbench_context_host/test_publisher.h"
#include "umicom/workbench_context_host/session_service.h"
#include "umicom/workbench_context_event/builders.h"
#include "umicom/workbench_context_event/service.h"

#define UMI_STUDIO_CONTEXT_GROUP_DEVELOPMENT "studio.context.development"
#define UMI_STUDIO_CONTEXT_GROUP_TESTING "studio.context.testing"
#define UMI_STUDIO_CONTEXT_GROUP_AI "studio.context.ai"
#define UMI_STUDIO_CONTEXT_GROUP_DATA "studio.context.data"

#define UMI_STUDIO_CONTEXT_ENDPOINT_OBSERVER "studio.context.observer"
#define UMI_STUDIO_CONTEXT_ENDPOINT_PROJECT "studio.context.project-explorer"
#define UMI_STUDIO_CONTEXT_ENDPOINT_EDITOR "studio.context.editor"
#define UMI_STUDIO_CONTEXT_ENDPOINT_PROBLEMS "studio.context.problems"
#define UMI_STUDIO_CONTEXT_ENDPOINT_SOURCE_CONTROL "studio.context.source-control"
#define UMI_STUDIO_CONTEXT_ENDPOINT_TESTS "studio.context.test-explorer"
#define UMI_STUDIO_CONTEXT_ENDPOINT_DEBUG "studio.context.debug"
#define UMI_STUDIO_CONTEXT_ENDPOINT_AI "studio.context.ai-assistant"
#define UMI_STUDIO_CONTEXT_ENDPOINT_DATA "studio.context.data-explorer"

struct UmiStudioContextLinkCentre {
    UmiUiWorkbench *workbench;
    UmiSessionStore *session;
    UmiWorkbenchContextLinkSlaveController link_controller;
    UmiWorkbenchContextHost *host;
    UmiWorkbenchContextHostSlaveController host_controller;
    UmiWorkbenchContextEventService *events;
    UmiWorkbenchContextHostProfile *profile;
    char workspace_id[UMI_WORKBENCH_CONTEXT_HOST_ID_CAPACITY];
    uint64_t publication_sequence;
    uint64_t last_refresh_ms;
};

/* Provide the mask operation used by this module and its client applications. */
static uint64_t mask(UmiContextKind kind)
{
    return umi_workbench_context_host_kind_mask(kind);
}

/* Provide the add group operation used by this module and its client applications. */
static UmiStatus add_group(
    UmiWorkbenchContextHostProfile *profile,
    const char *group_id,
    const char *title,
    UmiContextChannelColour colour,
    uint64_t kinds,
    bool active)
{
    UmiWorkbenchContextHostGroupDefinition group;
    UmiStatus status;
    umi_workbench_context_host_group_definition_init(&group, group_id);
    status = umi_workbench_context_host_copy_text(
        group.title, sizeof(group.title), title);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    group.colour = colour;
    group.allowed_kinds_mask = kinds;
    group.default_mode = UMI_WORKBENCH_CONTEXT_LINK_MODE_BIDIRECTIONAL;
    group.default_active = active;
    return umi_workbench_context_host_profile_add_group(profile, &group);
}

/* Provide the add endpoint operation used by this module and its client applications. */
static UmiStatus add_endpoint(
    UmiWorkbenchContextHostProfile *profile,
    const char *endpoint_id,
    const char *panel_id,
    const char *display_name,
    const char *group_id,
    UmiWorkbenchContextHostPanelRole role,
    UmiWorkbenchContextLinkMode mode,
    uint64_t accepted,
    uint64_t published)
{
    UmiWorkbenchContextHostEndpoint endpoint;
    UmiStatus status;
    umi_workbench_context_host_endpoint_init(&endpoint, endpoint_id);
    status = umi_workbench_context_host_endpoint_set_identity(
        &endpoint, panel_id, "org.umicom.studio", display_name);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_workbench_context_host_endpoint_set_group(
        &endpoint, group_id, mode);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    endpoint.role = role;
    endpoint.accepted_kinds_mask = accepted;
    endpoint.published_kinds_mask = published;
    endpoint.state = UMI_WORKBENCH_CONTEXT_HOST_ENDPOINT_ACTIVE;
    return umi_workbench_context_host_profile_add_endpoint(profile, &endpoint);
}

/* Provide the build profile operation used by this module and its client applications. */
static UmiStatus build_profile(UmiWorkbenchContextHostProfile *profile)
{
    const uint64_t source = mask(UMI_CONTEXT_KIND_SOURCE_LOCATION);
    const uint64_t project = mask(UMI_CONTEXT_KIND_PROJECT);
    const uint64_t workspace = mask(UMI_CONTEXT_KIND_WORKSPACE);
    const uint64_t selection = mask(UMI_CONTEXT_KIND_SELECTION);
    UmiStatus status;

    umi_workbench_context_host_profile_init(
        profile, "studio.context.profile", "org.umicom.studio");
    status = umi_workbench_context_host_profile_set_title(
        profile, "Umicom Studio Linked Workbench");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = add_group(
        profile,
        UMI_STUDIO_CONTEXT_GROUP_DEVELOPMENT,
        "Development",
        UMI_CONTEXT_COLOUR_BLUE,
        source | project | workspace | selection,
        true);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = add_group(
        profile,
        UMI_STUDIO_CONTEXT_GROUP_TESTING,
        "Testing",
        UMI_CONTEXT_COLOUR_GREEN,
        source | project | selection,
        false);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = add_group(
        profile,
        UMI_STUDIO_CONTEXT_GROUP_AI,
        "AI",
        UMI_CONTEXT_COLOUR_PURPLE,
        source | project | workspace | selection,
        false);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = add_group(
        profile,
        UMI_STUDIO_CONTEXT_GROUP_DATA,
        "Data",
        UMI_CONTEXT_COLOUR_CYAN,
        project | workspace | selection,
        false);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = add_endpoint(
        profile,
        UMI_STUDIO_CONTEXT_ENDPOINT_OBSERVER,
        "studio.workbench.observer",
        "Workbench",
        UMI_STUDIO_CONTEXT_GROUP_DEVELOPMENT,
        UMI_WORKBENCH_CONTEXT_HOST_PANEL_GENERIC,
        UMI_WORKBENCH_CONTEXT_LINK_MODE_PUBLISH,
        selection,
        selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = add_endpoint(
        profile,
        UMI_STUDIO_CONTEXT_ENDPOINT_PROJECT,
        "studio.project-explorer",
        "Project Explorer",
        UMI_STUDIO_CONTEXT_GROUP_DEVELOPMENT,
        UMI_WORKBENCH_CONTEXT_HOST_PANEL_EXPLORER,
        UMI_WORKBENCH_CONTEXT_LINK_MODE_BIDIRECTIONAL,
        project | workspace | selection,
        project | selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = add_endpoint(
        profile,
        UMI_STUDIO_CONTEXT_ENDPOINT_EDITOR,
        "studio.editor",
        "Editor",
        UMI_STUDIO_CONTEXT_GROUP_DEVELOPMENT,
        UMI_WORKBENCH_CONTEXT_HOST_PANEL_EDITOR,
        UMI_WORKBENCH_CONTEXT_LINK_MODE_BIDIRECTIONAL,
        source | project | selection,
        source | selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = add_endpoint(
        profile,
        UMI_STUDIO_CONTEXT_ENDPOINT_PROBLEMS,
        "studio.problems",
        "Problems",
        UMI_STUDIO_CONTEXT_GROUP_DEVELOPMENT,
        UMI_WORKBENCH_CONTEXT_HOST_PANEL_PROBLEMS,
        UMI_WORKBENCH_CONTEXT_LINK_MODE_BIDIRECTIONAL,
        source | project | selection,
        source | selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = add_endpoint(
        profile,
        UMI_STUDIO_CONTEXT_ENDPOINT_SOURCE_CONTROL,
        "studio.source-control",
        "Source Control",
        UMI_STUDIO_CONTEXT_GROUP_DEVELOPMENT,
        UMI_WORKBENCH_CONTEXT_HOST_PANEL_SOURCE_CONTROL,
        UMI_WORKBENCH_CONTEXT_LINK_MODE_BIDIRECTIONAL,
        project | source | selection,
        project | source | selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = add_endpoint(
        profile,
        UMI_STUDIO_CONTEXT_ENDPOINT_TESTS,
        "studio.test-explorer",
        "Test Explorer",
        UMI_STUDIO_CONTEXT_GROUP_TESTING,
        UMI_WORKBENCH_CONTEXT_HOST_PANEL_TEST_EXPLORER,
        UMI_WORKBENCH_CONTEXT_LINK_MODE_BIDIRECTIONAL,
        source | project | selection,
        source | selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = add_endpoint(
        profile,
        UMI_STUDIO_CONTEXT_ENDPOINT_DEBUG,
        "studio.debug",
        "Debugger",
        UMI_STUDIO_CONTEXT_GROUP_DEVELOPMENT,
        UMI_WORKBENCH_CONTEXT_HOST_PANEL_DEBUGGER,
        UMI_WORKBENCH_CONTEXT_LINK_MODE_BIDIRECTIONAL,
        source | project | selection,
        source | selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = add_endpoint(
        profile,
        UMI_STUDIO_CONTEXT_ENDPOINT_AI,
        "studio.ai",
        "AI Assistant",
        UMI_STUDIO_CONTEXT_GROUP_AI,
        UMI_WORKBENCH_CONTEXT_HOST_PANEL_AI,
        UMI_WORKBENCH_CONTEXT_LINK_MODE_BIDIRECTIONAL,
        source | project | workspace | selection,
        source | project | selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    return add_endpoint(
        profile,
        UMI_STUDIO_CONTEXT_ENDPOINT_DATA,
        "studio.data-explorer",
        "Data Explorer",
        UMI_STUDIO_CONTEXT_GROUP_DATA,
        UMI_WORKBENCH_CONTEXT_HOST_PANEL_EXPLORER,
        UMI_WORKBENCH_CONTEXT_LINK_MODE_BIDIRECTIONAL,
        project | workspace | selection,
        selection);
}


/*
 * Provide the register event source operation used by this module and its client
 * applications.
 */
static UmiStatus register_event_source(
    UmiStudioContextLinkCentre *centre,
    const char *source_id,
    const char *label)
{
    UmiWorkbenchContextEventSourceDescriptor source;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || centre->events == NULL ||
        source_id == NULL || label == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    umi_workbench_context_event_source_descriptor_init(
        &source,
        source_id);
    status = umi_workbench_context_event_source_descriptor_set_source(
        &source,
        source_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_workbench_context_event_source_descriptor_set_subject(
        &source,
        "org.umicom.studio");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_workbench_context_event_source_descriptor_set_label(
        &source,
        label);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    source.event_kind = UMI_WORKBENCH_CONTEXT_EVENT_GENERIC_SELECTION;
    source.context_kind = UMI_CONTEXT_KIND_SELECTION;
    source.state = UMI_WORKBENCH_CONTEXT_EVENT_ACCEPTED;
    return umi_workbench_context_event_service_register_source(
        centre->events,
        &source);
}

/*
 * Provide the register event sources operation used by this module and its client
 * applications.
 */
static UmiStatus register_event_sources(
    UmiStudioContextLinkCentre *centre)
{
    static const struct {
        const char *source_id;
        const char *label;
    } sources[] = {
        { "studio.project.selection", "Project Explorer selection" },
        { "studio.editor.location", "Editor caret and selection" },
        { "studio.problems.selection", "Problems selection" },
        { "studio.source-control.selection", "Source Control selection" },
        { "studio.test.selection", "Test Explorer selection" },
        { "studio.ai.selection", "AI selection" },
        { "studio.debug.location", "Debugger source location" },
        { "studio.workbench.selection", "Workbench selection" }
    };
    size_t index;
    UmiStatus status;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < sizeof(sources) / sizeof(sources[0]);
         ++index) {
        status = register_event_source(
            centre,
            sources[index].source_id,
            sources[index].label);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }
    return UMI_STATUS_OK;
}

/* Provide the submit event operation used by this module and its client applications. */
static UmiStatus submit_event(
    UmiStudioContextLinkCentre *centre,
    UmiWorkbenchContextEvent *event,
    const char *group_id)
{
    size_t processed = 0U;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || event == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /*
     * A NULL/empty group means "follow the panel's current Context Host
     * assignment". This is the normal path for real UI interactions and makes
     * user reassignment of a panel effective immediately. An explicit group
     * remains available for controlled programmatic publication.
     */
    if (group_id != NULL && group_id[0] != '\0') {
        status = umi_workbench_context_event_add_metadata(
            event,
            "group-id",
            group_id);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }

    status = umi_workbench_context_event_service_submit(
        centre->events,
        event);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    return umi_workbench_context_event_service_pump(
        centre->events,
        0U,
        &processed);
}

/*
 * Initialise studio context link centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_context_link_centre_create(
    UmiUiWorkbench *workbench,
    UmiSessionStore *session,
    UmiStudioContextLinkCentre **out_centre)
{
    UmiStudioContextLinkCentre *centre;
    UmiWorkbenchContextHostConfig config;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL || out_centre == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_centre = NULL;
    centre = (UmiStudioContextLinkCentre *)calloc(1U, sizeof(*centre));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    centre->profile = (UmiWorkbenchContextHostProfile *)calloc(
        1U, sizeof(*centre->profile));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre->profile == NULL) {
        free(centre);
        return UMI_STATUS_OUT_OF_MEMORY;
    }
    centre->workbench = workbench;
    centre->session = session;
    centre->publication_sequence = 1U;
    (void)umi_workbench_context_host_copy_text(
        centre->workspace_id,
        sizeof(centre->workspace_id),
        "studio.workspace");

    umi_workbench_context_link_slave_controller_init(
        &centre->link_controller);
    status = umi_workbench_context_link_slave_controller_start(
        &centre->link_controller);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_context_link_centre_destroy(centre);
        return status;
    }

    config = umi_workbench_context_host_config_default();
    config.host_id = "studio.workbench";
    config.application_id = "org.umicom.studio";
    config.observer_panel_id = "studio.workbench.observer";
    status = umi_workbench_context_host_create(
        &config,
        umi_workbench_context_link_slave_controller_service(
            &centre->link_controller),
        &centre->host);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_context_link_centre_destroy(centre);
        return status;
    }

    status = build_profile(centre->profile);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_workbench_context_host_apply_profile(
            centre->host, centre->profile);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_context_link_centre_destroy(centre);
        return status;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre->session != NULL) {
        bool restored = false;
        status = umi_workbench_context_host_session_restore(
            centre->host,
            centre->session,
            "studio.context-links",
            &restored);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) {
            umi_studio_context_link_centre_destroy(centre);
            return status;
        }
        (void)restored;
    }

    status = umi_workbench_context_event_service_create(
        centre->host,
        &centre->events);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = register_event_sources(centre);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_context_link_centre_destroy(centre);
        return status;
    }

    umi_workbench_context_host_slave_controller_init(
        &centre->host_controller, centre->host);
    status = umi_workbench_context_host_slave_controller_start(
        &centre->host_controller);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_context_link_centre_destroy(centre);
        return status;
    }

    *out_centre = centre;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio context link centre so the same storage can be
 * reused safely.
 */
void umi_studio_context_link_centre_destroy(
    UmiStudioContextLinkCentre *centre)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre->host != NULL) {
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (centre->session != NULL) {
            (void)umi_workbench_context_host_session_save(
                centre->host,
                centre->session,
                "studio.context-links");
        }
        (void)umi_workbench_context_host_slave_controller_stop(
            &centre->host_controller);
    }
    umi_workbench_context_event_service_destroy(centre->events);
    centre->events = NULL;

    umi_workbench_context_host_destroy(centre->host);
    centre->host = NULL;
    (void)umi_workbench_context_link_slave_controller_stop(
        &centre->link_controller);
    umi_workbench_context_link_slave_controller_destroy(
        &centre->link_controller);
    free(centre->profile);
    free(centre);
}

/*
 * Provide the studio context link centre refresh operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_context_link_centre_refresh(
    UmiStudioContextLinkCentre *centre,
    uint64_t now_ms)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || centre->host == NULL || centre->workbench == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_workbench_context_host_observe_workbench(
        centre->host, centre->workbench, now_ms);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) centre->last_refresh_ms = now_ms;
    return status;
}

/*
 * Provide the studio context link centre host operation used by this module and its client
 * applications.
 */
UmiWorkbenchContextHost *umi_studio_context_link_centre_host(
    UmiStudioContextLinkCentre *centre)
{
    return centre != NULL ? centre->host : NULL;
}

/* Provide the next context id operation used by this module and its client applications. */
static UmiStatus next_context_id(
    UmiStudioContextLinkCentre *centre,
    const char *prefix,
    char *out_id,
    size_t capacity)
{
    int written;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || prefix == NULL || out_id == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    written = snprintf(
        out_id,
        capacity,
        "%s-%llu",
        prefix,
        (unsigned long long)centre->publication_sequence++);
    return written >= 0 && (size_t)written < capacity
        ? UMI_STATUS_OK
        : UMI_STATUS_CAPACITY_EXCEEDED;
}

/*
 * Provide the studio context link centre publish project operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_context_link_centre_publish_project(
    UmiStudioContextLinkCentre *centre,
    const char *project_id,
    const char *repository_id,
    const char *root_path,
    const char *target_id,
    const char *configuration_id,
    const char *language_id,
    uint64_t now_ms)
{
    UmiWorkbenchContextEvent event;
    char context_id[UMI_WORKBENCH_CONTEXT_EVENT_ID_CAPACITY];
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = next_context_id(
        centre,
        "studio-project",
        context_id,
        sizeof(context_id));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = umi_workbench_context_event_build_project(
        &event,
        context_id,
        "studio.project.selection",
        "org.umicom.studio",
        "studio.project-explorer",
        centre->workspace_id,
        project_id,
        root_path,
        repository_id,
        language_id,
        now_ms);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = umi_workbench_context_event_add_metadata(
        &event, "target-id", target_id != NULL ? target_id : "");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_workbench_context_event_add_metadata(
        &event, "configuration-id",
        configuration_id != NULL ? configuration_id : "");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    return submit_event(
        centre,
        &event,
        NULL);
}

/*
 * Provide the studio context link centre publish source location operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_context_link_centre_publish_source_location(
    UmiStudioContextLinkCentre *centre,
    const char *file_path,
    const char *symbol,
    uint32_t line,
    uint32_t column,
    uint32_t selection_length,
    uint64_t now_ms)
{
    UmiWorkbenchContextEvent event;
    char context_id[UMI_WORKBENCH_CONTEXT_EVENT_ID_CAPACITY];
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = next_context_id(
        centre,
        "studio-source",
        context_id,
        sizeof(context_id));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = umi_workbench_context_event_build_editor_location(
        &event,
        context_id,
        "studio.editor.location",
        "org.umicom.studio",
        "studio.editor",
        centre->workspace_id,
        file_path,
        symbol != NULL ? symbol : "",
        line,
        column,
        selection_length,
        now_ms);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    return submit_event(
        centre,
        &event,
        NULL);
}

/*
 * Provide the studio context link centre publish diagnostic operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_context_link_centre_publish_diagnostic(
    UmiStudioContextLinkCentre *centre,
    const char *file_path,
    uint32_t line,
    uint32_t column,
    const char *severity,
    const char *diagnostic_code,
    const char *message,
    uint64_t now_ms)
{
    UmiWorkbenchContextEvent event;
    char context_id[UMI_WORKBENCH_CONTEXT_EVENT_ID_CAPACITY];
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = next_context_id(
        centre,
        "studio-diagnostic",
        context_id,
        sizeof(context_id));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = umi_workbench_context_event_build_diagnostic(
        &event,
        context_id,
        "studio.problems.selection",
        "org.umicom.studio",
        "studio.problems",
        centre->workspace_id,
        file_path,
        line,
        column,
        severity,
        diagnostic_code,
        message,
        now_ms);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    return submit_event(
        centre,
        &event,
        NULL);
}

/*
 * Provide the studio context link centre publish source control operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_context_link_centre_publish_source_control(
    UmiStudioContextLinkCentre *centre,
    const char *project_id,
    const char *root_path,
    const char *repository_id,
    const char *branch,
    const char *path,
    const char *change_kind,
    uint64_t now_ms)
{
    UmiWorkbenchContextEvent event;
    char context_id[UMI_WORKBENCH_CONTEXT_EVENT_ID_CAPACITY];
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = next_context_id(
        centre,
        "studio-vcs",
        context_id,
        sizeof(context_id));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = umi_workbench_context_event_build_source_control(
        &event,
        context_id,
        "studio.source-control.selection",
        "org.umicom.studio",
        "studio.source-control",
        centre->workspace_id,
        project_id,
        root_path,
        repository_id,
        branch,
        path,
        change_kind,
        now_ms);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    return submit_event(
        centre,
        &event,
        NULL);
}

/*
 * Provide the studio context link centre publish test operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_context_link_centre_publish_test(
    UmiStudioContextLinkCentre *centre,
    const char *test_id,
    const char *suite_id,
    const char *outcome,
    const char *source_uri,
    uint64_t source_line,
    uint64_t duration_ms,
    uint64_t now_ms)
{
    UmiWorkbenchContextEvent event;
    char context_id[UMI_WORKBENCH_CONTEXT_EVENT_ID_CAPACITY];
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = next_context_id(
        centre,
        "studio-test",
        context_id,
        sizeof(context_id));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = umi_workbench_context_event_build_test(
        &event,
        context_id,
        "studio.test.selection",
        "org.umicom.studio",
        "studio.test-explorer",
        centre->workspace_id,
        test_id,
        suite_id,
        outcome,
        source_uri,
        source_line,
        duration_ms,
        now_ms);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    return submit_event(
        centre,
        &event,
        NULL);
}

/*
 * Provide the studio context link centre publish ai operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_context_link_centre_publish_ai(
    UmiStudioContextLinkCentre *centre,
    const char *conversation_id,
    const char *message_id,
    const char *provider_id,
    const char *model_id,
    const char *evidence_id,
    uint64_t now_ms)
{
    UmiWorkbenchContextEvent event;
    char context_id[UMI_WORKBENCH_CONTEXT_EVENT_ID_CAPACITY];
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = next_context_id(
        centre,
        "studio-ai",
        context_id,
        sizeof(context_id));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = umi_workbench_context_event_build_ai(
        &event,
        context_id,
        "studio.ai.selection",
        "org.umicom.studio",
        "studio.ai",
        centre->workspace_id,
        message_id,
        conversation_id,
        provider_id,
        model_id,
        evidence_id,
        now_ms);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    return submit_event(
        centre,
        &event,
        NULL);
}

/*
 * Provide the studio context link centre publish debug location operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_context_link_centre_publish_debug_location(
    UmiStudioContextLinkCentre *centre,
    const char *file_path,
    const char *symbol,
    uint32_t line,
    uint32_t column,
    uint64_t now_ms)
{
    UmiWorkbenchContextEvent event;
    char context_id[UMI_WORKBENCH_CONTEXT_EVENT_ID_CAPACITY];
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = next_context_id(
        centre,
        "studio-debug",
        context_id,
        sizeof(context_id));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    status = umi_workbench_context_event_build_editor_location(
        &event,
        context_id,
        "studio.debug.location",
        "org.umicom.studio",
        "studio.debug",
        centre->workspace_id,
        file_path,
        symbol != NULL ? symbol : "",
        line,
        column,
        0U,
        now_ms);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    event.kind = UMI_WORKBENCH_CONTEXT_EVENT_DEBUG_LOCATION;
    (void)umi_workbench_context_event_refresh_hash(&event);

    return submit_event(
        centre,
        &event,
        NULL);
}

/*
 * Provide the studio context link centre publish selection operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_context_link_centre_publish_selection(
    UmiStudioContextLinkCentre *centre,
    const char *source_id,
    const char *panel_id,
    const char *subject_id,
    const char *selection_type,
    uint64_t now_ms)
{
    UmiWorkbenchContextEvent event;
    char context_id[UMI_WORKBENCH_CONTEXT_EVENT_ID_CAPACITY];
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || source_id == NULL || panel_id == NULL ||
        subject_id == NULL || selection_type == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    status = next_context_id(
        centre,
        "studio-selection",
        context_id,
        sizeof(context_id));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    umi_workbench_context_event_init(
        &event,
        UMI_WORKBENCH_CONTEXT_EVENT_GENERIC_SELECTION,
        context_id);
    event.context_kind = UMI_CONTEXT_KIND_SELECTION;
    event.timestamp_ms = now_ms;

    status = umi_workbench_context_event_copy_text(
        event.source_id,
        sizeof(event.source_id),
        source_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_workbench_context_event_copy_text(
        event.application_id,
        sizeof(event.application_id),
        "org.umicom.studio");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_workbench_context_event_copy_text(
        event.panel_id,
        sizeof(event.panel_id),
        panel_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_workbench_context_event_copy_text(
        event.workspace_id,
        sizeof(event.workspace_id),
        centre->workspace_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_workbench_context_event_copy_text(
        event.subject_id,
        sizeof(event.subject_id),
        subject_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_workbench_context_event_copy_text(
        event.secondary_id,
        sizeof(event.secondary_id),
        subject_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_workbench_context_event_add_metadata(
        &event,
        "selection-type",
        selection_type);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    /*
     * Do not force a colour group here.  The Framework Event Service resolves
     * the panel's current Context Host assignment, so user reassignment of a
     * panel immediately changes subsequent routing.
     */
    status = umi_workbench_context_event_service_submit(
        centre->events,
        &event);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    {
        size_t processed = 0U;
        return umi_workbench_context_event_service_pump(
            centre->events,
            0U,
            &processed);
    }
}
