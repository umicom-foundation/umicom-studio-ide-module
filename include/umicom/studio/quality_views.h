/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/quality_views.h
 *
 * PURPOSE:
 *   Publish Studio's public quality views contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_QUALITY_VIEWS_H
#define UMICOM_STUDIO_QUALITY_VIEWS_H

#include "umicom/studio/quality_centre.h"

/* View kinds are appended to preserve the numeric IDs of existing views. */
typedef enum UmiStudioQualityViewKind {
    UMI_STUDIO_QUALITY_VIEW_OVERVIEW = 1,
    UMI_STUDIO_QUALITY_VIEW_EVIDENCE,
    UMI_STUDIO_QUALITY_VIEW_ARCHITECTURE,
    UMI_STUDIO_QUALITY_VIEW_DUPLICATES,
    UMI_STUDIO_QUALITY_VIEW_COMPATIBILITY,
    UMI_STUDIO_QUALITY_VIEW_GATES,
    UMI_STUDIO_QUALITY_VIEW_REMEDIATION,
    UMI_STUDIO_QUALITY_VIEW_DOCUMENTATION
} UmiStudioQualityViewKind;

/* This toolkit-neutral view can be rendered by desktop or future web adapters. */
typedef struct UmiStudioQualityView {
    UmiStudioQualityViewKind kind; /* Selected presentation kind. */
    char id[UMI_CODEGUARD_QUALITY_ID_CAPACITY]; /* Stable command/view ID. */
    char title[UMI_CODEGUARD_QUALITY_ID_CAPACITY]; /* Human-readable title. */
    char summary[UMI_CODEGUARD_QUALITY_TEXT_CAPACITY]; /* Compact status text. */
    size_t item_count; /* Primary item count for badges and navigation. */
    uint64_t revision; /* Snapshot revision used to avoid stale rendering. */
} UmiStudioQualityView;

/* Return the immutable stable ID for a view kind. */
const char *umi_studio_quality_view_id(UmiStudioQualityViewKind kind);

/* Build one value-only view from current Framework quality evidence. */
UmiStatus umi_studio_quality_view_build(
    const UmiStudioQualityCentre *centre,
    UmiStudioQualityViewKind kind,
    UmiStudioQualityView *out_view);

#endif
