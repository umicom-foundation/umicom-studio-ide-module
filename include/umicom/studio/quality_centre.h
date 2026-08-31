/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/quality_centre.h
 *
 * PURPOSE:
 *   Publish Studio's public quality centre contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_QUALITY_CENTRE_H
#define UMICOM_STUDIO_QUALITY_CENTRE_H

#include "umicom/studio/services.h"
#include "umicom/codeguard/quality_session.h"

/* Studio adds only presentation state around the reusable Framework session. */
typedef struct UmiStudioQualityCentre {
    UmiStudioServices *services;  /* Borrowed application services. */
    UmiCodeGuardQualitySession session; /* Framework-owned quality behaviour. */
    char active_view[UMI_CODEGUARD_QUALITY_ID_CAPACITY]; /* Selected view ID. */
    uint64_t revision;            /* Studio presentation revision. */
} UmiStudioQualityCentre;

/* A snapshot owns plain values so UI adapters can render it after the call. */
typedef struct UmiStudioQualitySnapshot {
    size_t evidence;              /* General evidence records. */
    size_t failures;              /* Current quality failures. */
    size_t warnings;              /* Current quality warnings. */
    size_t duplicate_candidates;  /* Duplicate reviews. */
    size_t api_changes;           /* Public API differences. */
    size_t remediation_actions;   /* Suggested repair actions. */
    UmiCodeGuardQualityDecision decision; /* Current release decision. */
    char active_view[UMI_CODEGUARD_QUALITY_ID_CAPACITY]; /* Selected view ID. */
    uint64_t revision;            /* Presentation revision copied from centre. */
    /* Appended fields preserve all earlier member offsets. */
    size_t documentation_files;       /* Source files measured. */
    size_t documentation_failures;    /* Source files below their policy. */
    unsigned documentation_file_percent;     /* Passing-file percentage. */
    unsigned documentation_function_percent; /* Explained-function percentage. */
    unsigned documentation_decision_percent; /* Explained-decision percentage. */
} UmiStudioQualitySnapshot;

/* Create a caller-owned Quality Centre over borrowed Studio services. */
UmiStatus umi_studio_quality_centre_create(
    UmiStudioServices *services,
    UmiStudioQualityCentre **out_centre);

/* Release the centre; passing NULL is harmless. */
void umi_studio_quality_centre_destroy(UmiStudioQualityCentre *centre);

/* Select a named Quality Centre view and advance presentation revision. */
UmiStatus umi_studio_quality_centre_activate(
    UmiStudioQualityCentre *centre,
    const char *view_id);

/* Scan and aggregate one selected source file using Framework CodeGuard. */
UmiStatus umi_studio_quality_centre_review_documentation(
    UmiStudioQualityCentre *centre,
    const char *path,
    const UmiCodeGuardDocumentationPolicy *policy,
    UmiCodeGuardDocumentationReport *out_report);

/* Copy a stable value snapshot for toolkit-specific Quality Centre rendering. */
UmiStatus umi_studio_quality_centre_snapshot(
    const UmiStudioQualityCentre *centre,
    UmiStudioQualitySnapshot *out_snapshot);

#endif
