/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: include/umicom/studio/experience_centre.h
 *
 * PURPOSE:
 *   Expose Studio's product-readiness view over the canonical Framework application experience catalogue.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_EXPERIENCE_CENTRE_H
#define UMICOM_STUDIO_EXPERIENCE_CENTRE_H

#include <stddef.h>

#include "umicom/application/experience.h"
#include "umicom/application/experience_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio experience row data shared with callers of this public contract.
 */
typedef struct UmiStudioExperienceRow {
    const char *application_id;
    const char *display_name;
    const char *default_layout_id;
    size_t panel_count;
    size_t layout_count;
    size_t feature_count;
    unsigned readiness_percent;
    int has_unverified_p0;
} UmiStudioExperienceRow;

/**
 * Provide the studio experience operation used by this module and its client applications.
 */
const UmiApplicationExperienceDefinition *umi_studio_experience(void);

/**
 * Provide the studio experience status operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_experience_status(
    UmiApplicationExperienceStatus *out_status);

/**
 * Return the number of records represented by studio experience centre without changing
 * their state.
 */
size_t umi_studio_experience_centre_count(void);

/**
 * Provide the studio experience centre row operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_experience_centre_row(
    size_t index,
    UmiStudioExperienceRow *out_row);

/**
 * Provide the studio experience next feature operation used by this module and its client
 * applications.
 */
const UmiExperienceFeatureDefinition *umi_studio_experience_next_feature(void);

#ifdef __cplusplus
}
#endif

#endif
