/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: include/umicom/studio/application_runtime_centre.h
 *
 * PURPOSE:
 *   Expose canonical application runtime summaries in Studio without moving runtime ownership out of Framework.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_APPLICATION_RUNTIME_CENTRE_H
#define UMICOM_STUDIO_APPLICATION_RUNTIME_CENTRE_H

#include "umicom/application/runtime/runtime_catalogue.h"
#include "umicom/application_ui/summary_view.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t umi_studio_application_runtime_centre_count(void);
UmiStatus umi_studio_application_runtime_centre_row(
    size_t index,
    UmiApplicationUiSummaryRow *out_row);
UmiStatus umi_studio_application_runtime_catalogue_status(
    UmiApplicationRuntimeCatalogueStatus *out_status);

#ifdef __cplusplus
}
#endif

#endif
