/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/operations_views.h
 *
 * PURPOSE:
 *   Publish Studio's public operations views contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_OPERATIONS_VIEWS_H
#define UMICOM_STUDIO_OPERATIONS_VIEWS_H
#include "umicom/studio/operations_centre.h"
/**
 * Represent the studio operations view data shared with callers of this public contract.
 */
typedef struct UmiStudioOperationsView {
    char view_id[UMI_OPERATIONS_ID_CAPACITY];
    char title[UMI_OPERATIONS_NAME_CAPACITY];
    char summary[UMI_OPERATIONS_TEXT_CAPACITY];
    size_t item_count;
    bool available;
} UmiStudioOperationsView;
/**
 * Provide the studio operations view resolve operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_operations_view_resolve(const UmiStudioOperationsCentre *centre,const char *view_id,UmiStudioOperationsView *out_view);
#endif
