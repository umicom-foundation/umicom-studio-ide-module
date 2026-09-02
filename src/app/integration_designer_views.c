/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/integration_designer_views.c
 *
 * PURPOSE:
 *   Implement the integration designer views behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Integration Designer views | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/integration_designer_views.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/*
 * Provide the studio integration designer view id operation used by this module and its
 * client applications.
 */
const char *umi_studio_integration_designer_view_id(UmiStudioIntegrationDesignerViewKind kind)
{
    /* Select the behaviour associated with the requested command or state value. */
    switch (kind) {
        case UMI_STUDIO_INTEGRATION_VIEW_OVERVIEW: return "overview";
        case UMI_STUDIO_INTEGRATION_VIEW_CONNECTIONS: return "connections";
        case UMI_STUDIO_INTEGRATION_VIEW_API_EXPLORER: return "api-explorer";
        case UMI_STUDIO_INTEGRATION_VIEW_MAPPING: return "mapping";
        case UMI_STUDIO_INTEGRATION_VIEW_WORKFLOW: return "workflow";
        case UMI_STUDIO_INTEGRATION_VIEW_EXECUTION: return "execution";
        case UMI_STUDIO_INTEGRATION_VIEW_MONITORING: return "monitoring";
        default: return "unknown";
    }
}

/* Provide the view title operation used by this module and its client applications. */
static const char *view_title(UmiStudioIntegrationDesignerViewKind kind)
{
    /* Select the behaviour associated with the requested command or state value. */
    switch (kind) {
        case UMI_STUDIO_INTEGRATION_VIEW_OVERVIEW: return "Integration Fabric";
        case UMI_STUDIO_INTEGRATION_VIEW_CONNECTIONS: return "Connections";
        case UMI_STUDIO_INTEGRATION_VIEW_API_EXPLORER: return "API Explorer";
        case UMI_STUDIO_INTEGRATION_VIEW_MAPPING: return "Field Mapping";
        case UMI_STUDIO_INTEGRATION_VIEW_WORKFLOW: return "Workflow Designer";
        case UMI_STUDIO_INTEGRATION_VIEW_EXECUTION: return "Execution";
        case UMI_STUDIO_INTEGRATION_VIEW_MONITORING: return "Monitoring";
        default: return "Integration Designer";
    }
}

/*
 * Provide the studio integration designer view build operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_integration_designer_view_build(const UmiStudioIntegrationDesignerCentre *centre,UmiStudioIntegrationDesignerViewKind kind,UmiStudioIntegrationDesignerView *out_view)
{
    UmiStudioIntegrationDesignerSnapshot snapshot;
    int length;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || out_view == NULL || kind < UMI_STUDIO_INTEGRATION_VIEW_OVERVIEW || kind > UMI_STUDIO_INTEGRATION_VIEW_MONITORING) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_studio_integration_designer_snapshot(centre,&snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    (void)memset(out_view,0,sizeof(*out_view));
    out_view->kind = kind;
    status = umi_integration_designer_copy(out_view->id,sizeof(out_view->id),umi_studio_integration_designer_view_id(kind));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_integration_designer_copy(out_view->title,sizeof(out_view->title),view_title(kind));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    /* Select the behaviour associated with the requested command or state value. */
    switch (kind) {
        case UMI_STUDIO_INTEGRATION_VIEW_CONNECTIONS: out_view->item_count = snapshot.fabric.connections; break;
        case UMI_STUDIO_INTEGRATION_VIEW_API_EXPLORER: out_view->item_count = snapshot.fabric.contracts; break;
        case UMI_STUDIO_INTEGRATION_VIEW_MAPPING: out_view->item_count = snapshot.fabric.mappings; break;
        case UMI_STUDIO_INTEGRATION_VIEW_WORKFLOW: out_view->item_count = snapshot.fabric.workflows; break;
        case UMI_STUDIO_INTEGRATION_VIEW_EXECUTION:
        case UMI_STUDIO_INTEGRATION_VIEW_MONITORING: out_view->item_count = snapshot.fabric.runs.total; break;
        default: out_view->item_count = snapshot.fabric.connections + snapshot.fabric.contracts + snapshot.fabric.workflows; break;
    }
    length = snprintf(out_view->summary,sizeof(out_view->summary),"%s: %zu item(s), %zu successful run(s), revision %" PRIu64,out_view->title,out_view->item_count,snapshot.fabric.runs.succeeded,snapshot.revision);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length < 0 || (size_t)length >= sizeof(out_view->summary)) return UMI_STATUS_CAPACITY_EXCEEDED;
    out_view->revision = snapshot.revision;
    return UMI_STATUS_OK;
}
