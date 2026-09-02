/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/operations_centre.c
 *
 * PURPOSE:
 *   Implement the operations centre behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Operations Centre v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/operations_centre.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct UmiStudioOperationsCentre {
    UmiStudioServices *services;
    UmiOperationsCentre operations;
    char active_view[UMI_OPERATIONS_ID_CAPACITY];
    uint64_t revision;
};
/*
 * Initialise studio operations centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_operations_centre_create(UmiStudioServices *services,UmiStudioOperationsCentre **out_centre)
{
    UmiStudioOperationsCentre *centre;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || out_centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_centre = NULL;
    centre = (UmiStudioOperationsCentre *)calloc(1U,sizeof(*centre));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    centre->services = services;
    umi_operations_centre_init(&centre->operations);
    (void)snprintf(centre->active_view,sizeof(centre->active_view),"overview");
    centre->revision = 1U;
    *out_centre = centre;
    return UMI_STATUS_OK;
}
/*
 * Release or reset state held by studio operations centre so the same storage can be
 * reused safely.
 */
void umi_studio_operations_centre_destroy(UmiStudioOperationsCentre *centre) { free(centre); }
/*
 * Provide the studio operations centre activate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_operations_centre_activate(UmiStudioOperationsCentre *centre,const char *view_id)
{
    int length;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || view_id == NULL || umi_operations_catalogue_find(view_id) == NULL) return UMI_STATUS_NOT_FOUND;
    length = snprintf(centre->active_view,sizeof(centre->active_view),"%s",view_id);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (length < 0 || (size_t)length >= sizeof(centre->active_view)) return UMI_STATUS_CAPACITY_EXCEEDED;
    centre->revision += 1U;
    return UMI_STATUS_OK;
}
/*
 * Provide the studio operations centre snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_operations_centre_snapshot(const UmiStudioOperationsCentre *centre,UmiStudioOperationsCentreSnapshot *out_snapshot)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_snapshot,0,sizeof(*out_snapshot));
    umi_operations_centre_snapshot(&centre->operations,&out_snapshot->operations);
    (void)snprintf(out_snapshot->active_view,sizeof(out_snapshot->active_view),"%s",centre->active_view);
    out_snapshot->revision = centre->revision + out_snapshot->operations.revision;
    return UMI_STATUS_OK;
}
/*
 * Provide the studio operations centre engine operation used by this module and its client
 * applications.
 */
UmiOperationsCentre *umi_studio_operations_centre_engine(UmiStudioOperationsCentre *centre) { return centre == NULL ? NULL : &centre->operations; }
