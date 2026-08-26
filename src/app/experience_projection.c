/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: src/app/experience_projection.c
 *
 * PURPOSE:
 *   Format Framework product-readiness metadata without introducing reusable product logic into Studio.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/experience_projection.h"

#include <stdio.h>

UmiStatus umi_studio_experience_format_row(
    const UmiStudioExperienceRow *row,
    char *buffer,
    size_t buffer_size)
{
    int written;
    if (row == NULL || buffer == NULL || buffer_size == 0U)
        return UMI_STATUS_INVALID_ARGUMENT;
    written = snprintf(buffer, buffer_size,
        "%s — %u%% — %zu panels / %zu layouts / %zu features%s",
        row->display_name,
        row->readiness_percent,
        row->panel_count,
        row->layout_count,
        row->feature_count,
        row->has_unverified_p0 ? " — P0 pending" : "");
    if (written < 0) return UMI_STATUS_IO_ERROR;
    if ((size_t)written >= buffer_size) return UMI_STATUS_CAPACITY_EXCEEDED;
    return UMI_STATUS_OK;
}

UmiStatus umi_studio_experience_format_next_feature(
    const UmiExperienceFeatureDefinition *feature,
    char *buffer,
    size_t buffer_size)
{
    int written;
    if (feature == NULL || buffer == NULL || buffer_size == 0U)
        return UMI_STATUS_INVALID_ARGUMENT;
    written = snprintf(buffer, buffer_size,
        "%s [%s] — %s — owner: %s",
        feature->title,
        umi_experience_priority_text(feature->priority),
        umi_experience_feature_state_text(feature->state),
        umi_experience_ownership_text(feature->owner));
    if (written < 0) return UMI_STATUS_IO_ERROR;
    if ((size_t)written >= buffer_size) return UMI_STATUS_CAPACITY_EXCEEDED;
    return UMI_STATUS_OK;
}
