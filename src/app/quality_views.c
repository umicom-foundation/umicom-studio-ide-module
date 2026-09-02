/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/quality_views.c
 *
 * PURPOSE:
 *   Build toolkit-neutral Quality Centre views from reusable Framework
 *   evidence, including workspace source-guidance coverage.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/quality_views.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/* Convert a view enum into its stable command, persistence and navigation ID. */
const char *umi_studio_quality_view_id(UmiStudioQualityViewKind kind)
{
    /* Every known kind has one immutable ID; unknown values remain diagnosable. */
    switch (kind) {
        case UMI_STUDIO_QUALITY_VIEW_OVERVIEW:
            return "overview";
        case UMI_STUDIO_QUALITY_VIEW_EVIDENCE:
            return "evidence";
        case UMI_STUDIO_QUALITY_VIEW_ARCHITECTURE:
            return "architecture";
        case UMI_STUDIO_QUALITY_VIEW_DUPLICATES:
            return "duplicates";
        case UMI_STUDIO_QUALITY_VIEW_COMPATIBILITY:
            return "compatibility";
        case UMI_STUDIO_QUALITY_VIEW_GATES:
            return "gates";
        case UMI_STUDIO_QUALITY_VIEW_REMEDIATION:
            return "remediation";
        case UMI_STUDIO_QUALITY_VIEW_DOCUMENTATION:
            return "documentation";
        default:
            return "unknown";
    }
}

/* Return a concise beginner-readable title for one Quality Centre view. */
static const char *quality_view_title(UmiStudioQualityViewKind kind)
{
    /* Titles describe user outcomes rather than internal implementation names. */
    switch (kind) {
        case UMI_STUDIO_QUALITY_VIEW_OVERVIEW:
            return "Quality Centre";
        case UMI_STUDIO_QUALITY_VIEW_EVIDENCE:
            return "Quality Evidence";
        case UMI_STUDIO_QUALITY_VIEW_ARCHITECTURE:
            return "Architecture Boundaries";
        case UMI_STUDIO_QUALITY_VIEW_DUPLICATES:
            return "Duplicate Review";
        case UMI_STUDIO_QUALITY_VIEW_COMPATIBILITY:
            return "ABI and API Compatibility";
        case UMI_STUDIO_QUALITY_VIEW_GATES:
            return "Combined Quality Gates";
        case UMI_STUDIO_QUALITY_VIEW_REMEDIATION:
            return "Remediation Guidance";
        case UMI_STUDIO_QUALITY_VIEW_DOCUMENTATION:
            return "Source Guidance";
        default:
            return "Quality";
    }
}

/* Build one complete value view without exposing mutable Quality Centre state. */
UmiStatus umi_studio_quality_view_build(
    const UmiStudioQualityCentre *centre,
    UmiStudioQualityViewKind kind,
    UmiStudioQualityView *out_view)
{
    UmiStudioQualitySnapshot snapshot;
    int length;
    UmiStatus status;
    /* Reject unknown enum values before reading or writing caller-owned state. */
    if (centre == NULL || out_view == NULL ||
        kind < UMI_STUDIO_QUALITY_VIEW_OVERVIEW ||
        kind > UMI_STUDIO_QUALITY_VIEW_DOCUMENTATION) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_studio_quality_centre_snapshot(centre, &snapshot);
    /* Snapshot failure prevents a partially stale view from being published. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    (void)memset(out_view, 0, sizeof(*out_view));
    out_view->kind = kind;
    status = umi_codeguard_quality_copy(
        out_view->id, sizeof(out_view->id), umi_studio_quality_view_id(kind));
    /* Stable ID truncation would break commands and saved navigation state. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    status = umi_codeguard_quality_copy(
        out_view->title, sizeof(out_view->title), quality_view_title(kind));
    /* A complete title is required for accessible panel labelling. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    /* Choose the primary count used by badges and first-level navigation. */
    switch (kind) {
        case UMI_STUDIO_QUALITY_VIEW_EVIDENCE:
            out_view->item_count = snapshot.evidence;
            break;
        case UMI_STUDIO_QUALITY_VIEW_ARCHITECTURE:
            out_view->item_count = umi_codeguard_evidence_count_kind(
                &centre->session.evidence,
                UMI_CODEGUARD_EVIDENCE_ARCHITECTURE);
            break;
        case UMI_STUDIO_QUALITY_VIEW_DUPLICATES:
            out_view->item_count = snapshot.duplicate_candidates;
            break;
        case UMI_STUDIO_QUALITY_VIEW_COMPATIBILITY:
            out_view->item_count = snapshot.api_changes;
            break;
        case UMI_STUDIO_QUALITY_VIEW_GATES:
            out_view->item_count = snapshot.failures + snapshot.warnings;
            break;
        case UMI_STUDIO_QUALITY_VIEW_REMEDIATION:
            out_view->item_count = snapshot.remediation_actions;
            break;
        case UMI_STUDIO_QUALITY_VIEW_DOCUMENTATION:
            out_view->item_count = snapshot.documentation_files;
            break;
        default:
            out_view->item_count = snapshot.evidence +
                snapshot.duplicate_candidates + snapshot.api_changes;
            break;
    }
    /* Source Guidance uses coverage percentages that are more useful than decision text. */
    if (kind == UMI_STUDIO_QUALITY_VIEW_DOCUMENTATION) {
        length = snprintf(
            out_view->summary,
            sizeof(out_view->summary),
            "%s: %zu file(s), %zu below policy, functions %u%%, decisions %u%%",
            out_view->title,
            snapshot.documentation_files,
            snapshot.documentation_failures,
            snapshot.documentation_function_percent,
            snapshot.documentation_decision_percent);
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        length = snprintf(
            out_view->summary,
            sizeof(out_view->summary),
            "%s: %zu item(s), decision %s, revision %" PRIu64,
            out_view->title,
            out_view->item_count,
            umi_codeguard_quality_decision_text(snapshot.decision),
            snapshot.revision);
    }
    /* A truncated summary could hide a failure count or percentage from the user. */
    if (length < 0 || (size_t)length >= sizeof(out_view->summary)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    out_view->revision = snapshot.revision;
    return UMI_STATUS_OK;
}
