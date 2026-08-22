/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/context_link_centre.c
 *
 * PURPOSE:
 *   Create Studio's development, testing, AI and data context groups while delegating routing, history, pinning and delivery to Framework.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
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
    UmiWorkbenchContextHostProfile *profile;
    char workspace_id[UMI_WORKBENCH_CONTEXT_HOST_ID_CAPACITY];
    uint64_t publication_sequence;
    uint64_t last_refresh_ms;
};

static uint64_t mask(UmiContextKind kind)
{
    return umi_workbench_context_host_kind_mask(kind);
}

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
    if (status != UMI_STATUS_OK) return status;
    group.colour = colour;
    group.allowed_kinds_mask = kinds;
    group.default_mode = UMI_WORKBENCH_CONTEXT_LINK_MODE_BIDIRECTIONAL;
    group.default_active = active;
    return umi_workbench_context_host_profile_add_group(profile, &group);
}

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
    if (status != UMI_STATUS_OK) return status;
    status = umi_workbench_context_host_endpoint_set_group(
        &endpoint, group_id, mode);
    if (status != UMI_STATUS_OK) return status;
    endpoint.role = role;
    endpoint.accepted_kinds_mask = accepted;
    endpoint.published_kinds_mask = published;
    endpoint.state = UMI_WORKBENCH_CONTEXT_HOST_ENDPOINT_ACTIVE;
    return umi_workbench_context_host_profile_add_endpoint(profile, &endpoint);
}

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
    if (status != UMI_STATUS_OK) return status;

    status = add_group(
        profile,
        UMI_STUDIO_CONTEXT_GROUP_DEVELOPMENT,
        "Development",
        UMI_CONTEXT_COLOUR_BLUE,
        source | project | workspace | selection,
        true);
    if (status != UMI_STATUS_OK) return status;
    status = add_group(
        profile,
        UMI_STUDIO_CONTEXT_GROUP_TESTING,
        "Testing",
        UMI_CONTEXT_COLOUR_GREEN,
        source | project | selection,
        false);
    if (status != UMI_STATUS_OK) return status;
    status = add_group(
        profile,
        UMI_STUDIO_CONTEXT_GROUP_AI,
        "AI",
        UMI_CONTEXT_COLOUR_PURPLE,
        source | project | workspace | selection,
        false);
    if (status != UMI_STATUS_OK) return status;
    status = add_group(
        profile,
        UMI_STUDIO_CONTEXT_GROUP_DATA,
        "Data",
        UMI_CONTEXT_COLOUR_CYAN,
        project | workspace | selection,
        false);
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
    if (status != UMI_STATUS_OK) return status;

    status = add_endpoint(
        profile,
        UMI_STUDIO_CONTEXT_ENDPOINT_TESTS,
        "studio.test-explorer",
        "Test Explorer",
        UMI_STUDIO_CONTEXT_GROUP_DEVELOPMENT,
        UMI_WORKBENCH_CONTEXT_HOST_PANEL_TEST_EXPLORER,
        UMI_WORKBENCH_CONTEXT_LINK_MODE_BIDIRECTIONAL,
        source | project | selection,
        source | selection);
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
    if (status != UMI_STATUS_OK) return status;

    status = add_endpoint(
        profile,
        UMI_STUDIO_CONTEXT_ENDPOINT_AI,
        "studio.ai",
        "AI Assistant",
        UMI_STUDIO_CONTEXT_GROUP_DEVELOPMENT,
        UMI_WORKBENCH_CONTEXT_HOST_PANEL_AI,
        UMI_WORKBENCH_CONTEXT_LINK_MODE_BIDIRECTIONAL,
        source | project | workspace | selection,
        source | project | selection);
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

UmiStatus umi_studio_context_link_centre_create(
    UmiUiWorkbench *workbench,
    UmiSessionStore *session,
    UmiStudioContextLinkCentre **out_centre)
{
    UmiStudioContextLinkCentre *centre;
    UmiWorkbenchContextHostConfig config;
    UmiStatus status;

    if (workbench == NULL || out_centre == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_centre = NULL;
    centre = (UmiStudioContextLinkCentre *)calloc(1U, sizeof(*centre));
    if (centre == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    centre->profile = (UmiWorkbenchContextHostProfile *)calloc(
        1U, sizeof(*centre->profile));
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
    if (status != UMI_STATUS_OK) {
        umi_studio_context_link_centre_destroy(centre);
        return status;
    }

    status = build_profile(centre->profile);
    if (status == UMI_STATUS_OK) {
        status = umi_workbench_context_host_apply_profile(
            centre->host, centre->profile);
    }
    if (status != UMI_STATUS_OK) {
        umi_studio_context_link_centre_destroy(centre);
        return status;
    }

    if (centre->session != NULL) {
        bool restored = false;
        status = umi_workbench_context_host_session_restore(
            centre->host,
            centre->session,
            "studio.context-links",
            &restored);
        if (status != UMI_STATUS_OK) {
            umi_studio_context_link_centre_destroy(centre);
            return status;
        }
        (void)restored;
    }

    umi_workbench_context_host_slave_controller_init(
        &centre->host_controller, centre->host);
    status = umi_workbench_context_host_slave_controller_start(
        &centre->host_controller);
    if (status != UMI_STATUS_OK) {
        umi_studio_context_link_centre_destroy(centre);
        return status;
    }

    *out_centre = centre;
    return UMI_STATUS_OK;
}

void umi_studio_context_link_centre_destroy(
    UmiStudioContextLinkCentre *centre)
{
    if (centre == NULL) return;
    if (centre->host != NULL) {
        if (centre->session != NULL) {
            (void)umi_workbench_context_host_session_save(
                centre->host,
                centre->session,
                "studio.context-links");
        }
        (void)umi_workbench_context_host_slave_controller_stop(
            &centre->host_controller);
    }
    umi_workbench_context_host_destroy(centre->host);
    centre->host = NULL;
    (void)umi_workbench_context_link_slave_controller_stop(
        &centre->link_controller);
    umi_workbench_context_link_slave_controller_destroy(
        &centre->link_controller);
    free(centre->profile);
    free(centre);
}

UmiStatus umi_studio_context_link_centre_refresh(
    UmiStudioContextLinkCentre *centre,
    uint64_t now_ms)
{
    UmiStatus status;
    if (centre == NULL || centre->host == NULL || centre->workbench == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_workbench_context_host_observe_workbench(
        centre->host, centre->workbench, now_ms);
    if (status == UMI_STATUS_OK) centre->last_refresh_ms = now_ms;
    return status;
}

UmiWorkbenchContextHost *umi_studio_context_link_centre_host(
    UmiStudioContextLinkCentre *centre)
{
    return centre != NULL ? centre->host : NULL;
}

static const char *group_for_panel(
    const UmiStudioContextLinkCentre *centre,
    const char *panel_id)
{
    const UmiWorkbenchContextHostEndpoint *endpoint;
    if (centre == NULL || centre->host == NULL || panel_id == NULL) {
        return NULL;
    }
    endpoint = umi_workbench_context_host_endpoint_registry_find_panel(
        &centre->host->endpoints, panel_id);
    if (endpoint == NULL || endpoint->group_id[0] == '\0') {
        return NULL;
    }
    return endpoint->group_id;
}

static UmiStatus next_context_id(
    UmiStudioContextLinkCentre *centre,
    const char *prefix,
    char *out_id,
    size_t capacity)
{
    int written;
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
    char context_id[UMI_WORKBENCH_CONTEXT_HOST_ID_CAPACITY];
    UmiStatus status;
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = next_context_id(
        centre, "studio-project", context_id, sizeof(context_id));
    if (status != UMI_STATUS_OK) return status;
    return umi_workbench_context_host_publish_project(
        centre->host,
        group_for_panel(centre, "studio.project-explorer"),
        "studio.project-explorer",
        context_id,
        project_id,
        repository_id,
        root_path,
        target_id,
        configuration_id,
        language_id,
        now_ms);
}

UmiStatus umi_studio_context_link_centre_publish_source_location(
    UmiStudioContextLinkCentre *centre,
    const char *file_path,
    const char *symbol,
    uint32_t line,
    uint32_t column,
    uint32_t selection_length,
    uint64_t now_ms)
{
    char context_id[UMI_WORKBENCH_CONTEXT_HOST_ID_CAPACITY];
    UmiStatus status;
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = next_context_id(
        centre, "studio-source", context_id, sizeof(context_id));
    if (status != UMI_STATUS_OK) return status;
    return umi_workbench_context_host_publish_source_location(
        centre->host,
        group_for_panel(centre, "studio.editor"),
        "studio.editor",
        context_id,
        centre->workspace_id,
        file_path,
        symbol != NULL ? symbol : "",
        line,
        column,
        selection_length,
        now_ms);
}

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
    char context_id[UMI_WORKBENCH_CONTEXT_HOST_ID_CAPACITY];
    UmiStatus status;
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = next_context_id(
        centre, "studio-diagnostic", context_id, sizeof(context_id));
    if (status != UMI_STATUS_OK) return status;
    return umi_workbench_context_host_publish_diagnostic(
        centre->host,
        group_for_panel(centre, "studio.problems"),
        "studio.problems",
        context_id,
        centre->workspace_id,
        file_path,
        line,
        column,
        severity,
        diagnostic_code,
        message,
        now_ms);
}

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
    char context_id[UMI_WORKBENCH_CONTEXT_HOST_ID_CAPACITY];
    UmiStatus status;
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = next_context_id(
        centre, "studio-test", context_id, sizeof(context_id));
    if (status != UMI_STATUS_OK) return status;
    return umi_workbench_context_host_publish_test(
        centre->host,
        group_for_panel(centre, "studio.test-explorer"),
        "studio.test-explorer",
        context_id,
        test_id,
        suite_id,
        outcome,
        source_uri,
        source_line,
        duration_ms,
        now_ms);
}

UmiStatus umi_studio_context_link_centre_publish_ai(
    UmiStudioContextLinkCentre *centre,
    const char *conversation_id,
    const char *message_id,
    const char *provider_id,
    const char *model_id,
    const char *evidence_id,
    uint64_t now_ms)
{
    char context_id[UMI_WORKBENCH_CONTEXT_HOST_ID_CAPACITY];
    UmiStatus status;
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = next_context_id(
        centre, "studio-ai", context_id, sizeof(context_id));
    if (status != UMI_STATUS_OK) return status;
    return umi_workbench_context_host_publish_ai(
        centre->host,
        group_for_panel(centre, "studio.ai"),
        "studio.ai",
        context_id,
        conversation_id,
        message_id,
        provider_id,
        model_id,
        evidence_id,
        now_ms);
}
