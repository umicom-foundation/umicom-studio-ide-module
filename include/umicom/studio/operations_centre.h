/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/operations_centre.h
 *
 * PURPOSE:
 *   Publish Studio's public operations centre contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_OPERATIONS_CENTRE_H
#define UMICOM_STUDIO_OPERATIONS_CENTRE_H
#include "umicom/studio/services.h"
#include "umicom/observability/observability_admin.h"
typedef struct UmiStudioOperationsCentre UmiStudioOperationsCentre;
typedef struct UmiStudioOperationsCentreSnapshot {
    UmiOperationsCentreSnapshot operations;
    char active_view[UMI_OPERATIONS_ID_CAPACITY];
    uint64_t revision;
} UmiStudioOperationsCentreSnapshot;
UmiStatus umi_studio_operations_centre_create(UmiStudioServices *services,UmiStudioOperationsCentre **out_centre);
void umi_studio_operations_centre_destroy(UmiStudioOperationsCentre *centre);
UmiStatus umi_studio_operations_centre_activate(UmiStudioOperationsCentre *centre,const char *view_id);
UmiStatus umi_studio_operations_centre_snapshot(const UmiStudioOperationsCentre *centre,UmiStudioOperationsCentreSnapshot *out_snapshot);
UmiOperationsCentre *umi_studio_operations_centre_engine(UmiStudioOperationsCentre *centre);
#endif
