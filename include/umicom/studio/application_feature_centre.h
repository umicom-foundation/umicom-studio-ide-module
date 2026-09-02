/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: include/umicom/studio/application_feature_centre.h
 *
 * PURPOSE:
 *   Expose the canonical incomplete product backlog and ownership evidence in Studio.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_APPLICATION_FEATURE_CENTRE_H
#define UMICOM_STUDIO_APPLICATION_FEATURE_CENTRE_H

#include "umicom/application_ui/feature_backlog_view.h"
#include "umicom/application_ui/ownership_view.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return the number of records represented by studio application feature backlog without
 * changing their state.
 */
size_t umi_studio_application_feature_backlog_count(
    const char *application_id);
/**
 * Provide the studio application feature backlog row operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_feature_backlog_row(
    const char *application_id,
    size_t index,
    UmiApplicationFeatureProjection *out_row);
/**
 * Provide the studio application ownership rows operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_application_ownership_rows(
    const char *application_id,
    UmiApplicationUiOwnershipRow rows[UMI_APPLICATION_UI_OWNERSHIP_ROW_COUNT]);

#ifdef __cplusplus
}
#endif

#endif
