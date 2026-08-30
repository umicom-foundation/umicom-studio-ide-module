/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: include/umicom/studio/experience_projection.h
 *
 * PURPOSE:
 *   Project Framework product-readiness metadata into deterministic Studio-facing text rows.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_EXPERIENCE_PROJECTION_H
#define UMICOM_STUDIO_EXPERIENCE_PROJECTION_H

#include <stddef.h>

#include "umicom/studio/experience_centre.h"

#ifdef __cplusplus
extern "C" {
#endif

UmiStatus umi_studio_experience_format_row(
    const UmiStudioExperienceRow *row,
    char *buffer,
    size_t buffer_size);

UmiStatus umi_studio_experience_format_next_feature(
    const UmiExperienceFeatureDefinition *feature,
    char *buffer,
    size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
