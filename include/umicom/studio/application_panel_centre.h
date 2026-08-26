/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: include/umicom/studio/application_panel_centre.h
 *
 * PURPOSE:
 *   Expose Framework panel and layout projections for application inspection in Studio.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_APPLICATION_PANEL_CENTRE_H
#define UMICOM_STUDIO_APPLICATION_PANEL_CENTRE_H

#include "umicom/application_ui/layout_selector_view.h"
#include "umicom/application_ui/panel_catalogue_view.h"

#ifdef __cplusplus
extern "C" {
#endif

UmiStatus umi_studio_application_panel_row(
    const char *application_id,
    size_t index,
    UmiApplicationPanelProjection *out_row);
UmiStatus umi_studio_application_layout_row(
    const char *application_id,
    size_t index,
    UmiApplicationLayoutProjection *out_row);

#ifdef __cplusplus
}
#endif

#endif
