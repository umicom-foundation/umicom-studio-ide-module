/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/web_api_workbench_contribution.c
 *
 * PURPOSE:
 *   Place Framework HTTP/API/Web/Cloud commands and views in Studio. This file
 *   intentionally contains no request, authentication, cloud or deployment logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/web_api_workbench_contribution.h"

#include <string.h>

#define COMMAND(id, menu_name, group_name, order)                             \
    {                                                                          \
        (uint32_t)sizeof(UmiStudioWebApiWorkbenchCommandContribution),         \
        UMI_STUDIO_WEB_API_WORKBENCH_CONTRIBUTION_API_VERSION,                 \
        id, menu_name, group_name, order, true                                 \
    }

static const UmiStudioWebApiWorkbenchCommandContribution COMMANDS[] = {
    COMMAND("web.workbench.request.new", "File", "api-request", 100),
    COMMAND("web.workbench.request.save", "File", "api-request", 110),
    COMMAND("web.workbench.request.duplicate", "Edit", "api-request", 120),
    COMMAND("web.workbench.request.delete", "Edit", "api-request", 130),
    COMMAND("web.workbench.request.send", "Run", "api-request", 140),
    COMMAND("web.workbench.request.cancel", "Run", "api-request", 150),
    COMMAND("web.workbench.environment.new", "Web", "environment", 200),
    COMMAND("web.workbench.environment.edit", "Web", "environment", 210),
    COMMAND("web.workbench.environment.select", "Web", "environment", 220),
    COMMAND("web.workbench.auth.new", "Web", "authentication", 300),
    COMMAND("web.workbench.auth.edit", "Web", "authentication", 310),
    COMMAND("web.workbench.collection.import", "File", "api-collection", 400),
    COMMAND("web.workbench.collection.export", "File", "api-collection", 410),
    COMMAND("web.workbench.collection.run", "Run", "api-collection", 420),
    COMMAND("web.workbench.history.clear", "Web", "history", 500),
    COMMAND("web.workbench.history.replay", "Run", "history", 510),
    COMMAND("web.workbench.openapi.import", "Web", "openapi", 600),
    COMMAND("web.workbench.openapi.refresh", "Web", "openapi", 610),
    COMMAND("web.workbench.openapi.filter", "Navigate", "openapi", 620),
    COMMAND("web.workbench.openapi.generate-request", "Web", "openapi", 630),
    COMMAND("web.workbench.soap.new", "Web", "soap", 700),
    COMMAND("web.workbench.soap.send", "Run", "soap", 710),
    COMMAND("web.workbench.websocket.connect", "Web", "websocket", 800),
    COMMAND("web.workbench.websocket.disconnect", "Web", "websocket", 810),
    COMMAND("web.workbench.websocket.send", "Run", "websocket", 820),
    COMMAND("web.workbench.websocket.clear", "Web", "websocket", 830),
    COMMAND("web.workbench.sse.connect", "Web", "sse", 900),
    COMMAND("web.workbench.sse.disconnect", "Web", "sse", 910),
    COMMAND("web.workbench.sse.clear", "Web", "sse", 920),
    COMMAND("web.workbench.server.open", "View", "web-server", 1000),
    COMMAND("web.workbench.server.refresh", "Web", "web-server", 1010),
    COMMAND("web.workbench.server.start", "Run", "web-server", 1020),
    COMMAND("web.workbench.server.stop", "Run", "web-server", 1030),
    COMMAND("web.workbench.cloud.profile.new", "Cloud", "profile", 1100),
    COMMAND("web.workbench.cloud.profile.edit", "Cloud", "profile", 1110),
    COMMAND("web.workbench.cloud.objects.refresh", "Cloud", "objects", 1200),
    COMMAND("web.workbench.cloud.objects.upload", "Cloud", "objects", 1210),
    COMMAND("web.workbench.cloud.objects.download", "Cloud", "objects", 1220),
    COMMAND("web.workbench.cloud.objects.delete", "Cloud", "objects", 1230),
    COMMAND("web.workbench.cloud.queues.refresh", "Cloud", "queues", 1300),
    COMMAND("web.workbench.cloud.queues.send", "Cloud", "queues", 1310),
    COMMAND("web.workbench.cloud.queues.peek", "Cloud", "queues", 1320),
    COMMAND("web.workbench.cloud.queues.purge", "Cloud", "queues", 1330),
    COMMAND("web.workbench.deployment.open", "View", "deployment", 1400),
    COMMAND("web.workbench.deployment.deploy", "Run", "deployment", 1410),
    COMMAND("web.workbench.deployment.rollback", "Run", "deployment", 1420)
};

#undef COMMAND

#define VIEW(id, view_label, contract, placement, order)                      \
    {                                                                          \
        (uint32_t)sizeof(UmiStudioWebApiWorkbenchViewContribution),            \
        UMI_STUDIO_WEB_API_WORKBENCH_CONTRIBUTION_API_VERSION,                 \
        id, view_label, contract, placement, order, true, true                 \
    }

static const UmiStudioWebApiWorkbenchViewContribution VIEWS[] = {
    VIEW("studio.web-api.collections", "API Collections",
        "umicom.web.workbench.collection", "left-sidebar", 500),
    VIEW("studio.web-api.environments", "API Environments",
        "umicom.web.workbench.environment", "left-sidebar", 510),
    VIEW("studio.web-api.rest-client", "REST Client",
        "umicom.web.workbench.request", "editor-area", 520),
    VIEW("studio.web-api.response", "Response Inspector",
        "umicom.web.workbench.response", "bottom-panel", 530),
    VIEW("studio.web-api.openapi", "OpenAPI Explorer",
        "umicom.web.workbench.openapi", "secondary-sidebar", 540),
    VIEW("studio.web-api.soap", "XML and SOAP",
        "umicom.web.workbench.soap", "editor-area", 550),
    VIEW("studio.web-api.websocket", "WebSocket Messages",
        "umicom.web.workbench.stream", "bottom-panel", 560),
    VIEW("studio.web-api.sse", "Server-Sent Events",
        "umicom.web.workbench.stream", "bottom-panel", 570),
    VIEW("studio.web-api.server", "Web Server Diagnostics",
        "umicom.web.workbench.server-diagnostics", "bottom-panel", 580),
    VIEW("studio.web-api.cloud-objects", "Cloud Objects",
        "umicom.web.workbench.cloud-object", "left-sidebar", 590),
    VIEW("studio.web-api.cloud-queues", "Cloud Queues",
        "umicom.web.workbench.cloud-queue", "left-sidebar", 600),
    VIEW("studio.web-api.deployment", "Web Deployment Targets",
        "umicom.web.workbench.deployment-projection", "bottom-panel", 610),
    VIEW("studio.web-api.history", "API Request History",
        "umicom.web.workbench.history", "bottom-panel", 620)
};

#undef VIEW

/*
 * Return the number of records represented by studio web api workbench command without
 * changing their state.
 */
size_t umi_studio_web_api_workbench_command_count(void)
{
    return sizeof(COMMANDS) / sizeof(COMMANDS[0]);
}

/*
 * Find studio web api workbench command while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioWebApiWorkbenchCommandContribution *
umi_studio_web_api_workbench_command_at(size_t index)
{
    return index < umi_studio_web_api_workbench_command_count()
        ? &COMMANDS[index] : NULL;
}

/*
 * Find studio web api workbench command while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioWebApiWorkbenchCommandContribution *
umi_studio_web_api_workbench_command_find(const char *framework_command_id)
{
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (framework_command_id == NULL) return NULL;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < umi_studio_web_api_workbench_command_count(); ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(COMMANDS[index].framework_command_id, framework_command_id) == 0) {
            return &COMMANDS[index];
        }
    }
    return NULL;
}

/*
 * Return the number of records represented by studio web api workbench view without
 * changing their state.
 */
size_t umi_studio_web_api_workbench_view_count(void)
{
    return sizeof(VIEWS) / sizeof(VIEWS[0]);
}

/*
 * Find studio web api workbench view while leaving the underlying catalogue or model owned
 * by this module.
 */
const UmiStudioWebApiWorkbenchViewContribution *
umi_studio_web_api_workbench_view_at(size_t index)
{
    return index < umi_studio_web_api_workbench_view_count() ? &VIEWS[index] : NULL;
}

/*
 * Find studio web api workbench view while leaving the underlying catalogue or model owned
 * by this module.
 */
const UmiStudioWebApiWorkbenchViewContribution *
umi_studio_web_api_workbench_view_find(const char *view_id)
{
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (view_id == NULL) return NULL;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < umi_studio_web_api_workbench_view_count(); ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(VIEWS[index].view_id, view_id) == 0) return &VIEWS[index];
    }
    return NULL;
}

/*
 * Initialise studio web api workbench from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_web_api_workbench_create(
    UmiWebWorkbenchRuntime **out_runtime)
{
    return umi_web_workbench_runtime_create(out_runtime);
}
