/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: include/umicom/studio/application_runtime_centre.h
 *
 * PURPOSE:
 *   Expose canonical application runtime summaries in Studio without moving runtime ownership out of Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_APPLICATION_RUNTIME_CENTRE_H
#define UMICOM_STUDIO_APPLICATION_RUNTIME_CENTRE_H

#include "umicom/application/runtime/runtime_catalogue.h"
#include "umicom/application_ui/summary_view.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return the number of records represented by studio application runtime centre without
 * changing their state.
 */
size_t umi_studio_application_runtime_centre_count(void);
/**
 * Provide the studio application runtime centre row operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_runtime_centre_row(
    size_t index,
    UmiApplicationUiSummaryRow *out_row);
/**
 * Provide the studio application runtime catalogue status operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_application_runtime_catalogue_status(
    UmiApplicationRuntimeCatalogueStatus *out_status);

#ifdef __cplusplus
}
#endif

#endif
