/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/quality_centre.c
 *
 * PURPOSE:
 *   Present reusable Framework quality evidence, including source guidance,
 *   through a small toolkit-neutral Studio application facade.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/quality_centre.h"

#include <stdlib.h>
#include <string.h>

/* Allocate the thin Studio facade and initialise its Framework quality session. */
UmiStatus umi_studio_quality_centre_create(
    UmiStudioServices *services,
    UmiStudioQualityCentre **out_centre)
{
    UmiStudioQualityCentre *centre;
    UmiStatus status;
    /* Borrowed services and an output location are both required. */
    if (services == NULL || out_centre == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_centre = NULL;
    centre = (UmiStudioQualityCentre *)calloc(1U, sizeof(*centre));
    /* Allocation failure leaves the caller's output predictably NULL. */
    if (centre == NULL) {
        return UMI_STATUS_OUT_OF_MEMORY;
    }
    centre->services = services;
    umi_codeguard_quality_session_init(&centre->session);
    status = umi_codeguard_quality_copy(
        centre->active_view, sizeof(centre->active_view), "overview");
    /* Release owned memory when the initial stable view ID could not be stored. */
    if (status != UMI_STATUS_OK) {
        free(centre);
        return status;
    }
    centre->revision = 1U;
    *out_centre = centre;
    return UMI_STATUS_OK;
}

/* Release only the facade because its nested Framework session owns no heap pointer. */
void umi_studio_quality_centre_destroy(UmiStudioQualityCentre *centre)
{
    free(centre);
}

/* Select one stable view ID without coupling selection state to a UI toolkit. */
UmiStatus umi_studio_quality_centre_activate(
    UmiStudioQualityCentre *centre,
    const char *view_id)
{
    UmiStatus status;
    /* Empty IDs cannot identify a command, navigation target or saved view. */
    if (centre == NULL || view_id == NULL || view_id[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_codeguard_quality_copy(
        centre->active_view, sizeof(centre->active_view), view_id);
    /* Publish a presentation revision only after the complete ID was copied. */
    if (status == UMI_STATUS_OK) {
        centre->revision += 1U;
    }
    return status;
}

/* Scan one file in Framework, then stream the report into the shared session. */
UmiStatus umi_studio_quality_centre_review_documentation(
    UmiStudioQualityCentre *centre,
    const char *path,
    const UmiCodeGuardDocumentationPolicy *policy,
    UmiCodeGuardDocumentationReport *out_report)
{
    UmiStatus status;
    /* The centre owns the target session; Framework validates the other arguments. */
    if (centre == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_codeguard_documentation_scan_file(path, policy, out_report);
    /* A partial or unreadable report must never enter workspace totals. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    status = umi_codeguard_quality_session_record_documentation(
        &centre->session, out_report);
    /* Successful aggregation changes the observable Quality Centre content. */
    if (status == UMI_STATUS_OK) {
        centre->revision += 1U;
    }
    return status;
}

/* Copy all display values so adapters never borrow mutable session internals. */
UmiStatus umi_studio_quality_centre_snapshot(
    const UmiStudioQualityCentre *centre,
    UmiStudioQualitySnapshot *out_snapshot)
{
    const UmiCodeGuardDocumentationInventory *documentation;
    /* Both the source centre and caller-owned destination are mandatory. */
    if (centre == NULL || out_snapshot == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->evidence = centre->session.evidence.count;
    out_snapshot->failures = centre->session.evaluation.failures;
    out_snapshot->warnings = centre->session.evaluation.warnings;
    out_snapshot->duplicate_candidates = centre->session.duplicates.count;
    out_snapshot->api_changes = centre->session.api.count;
    out_snapshot->remediation_actions = centre->session.remediation.count;
    out_snapshot->decision = centre->session.evaluation.decision;
    out_snapshot->revision = centre->revision;
    documentation = &centre->session.documentation;
    /* Reject corrupted public session counters before calculating a subtraction. */
    if (umi_codeguard_documentation_inventory_validate(documentation) !=
        UMI_STATUS_OK) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    out_snapshot->documentation_files = documentation->file_count;
    out_snapshot->documentation_failures =
        documentation->file_count - documentation->passing_file_count;
    out_snapshot->documentation_file_percent =
        umi_codeguard_documentation_inventory_file_percent(documentation);
    out_snapshot->documentation_function_percent =
        umi_codeguard_documentation_inventory_function_percent(documentation);
    out_snapshot->documentation_decision_percent =
        umi_codeguard_documentation_inventory_decision_percent(documentation);
    return umi_codeguard_quality_copy(
        out_snapshot->active_view,
        sizeof(out_snapshot->active_view),
        centre->active_view);
}
