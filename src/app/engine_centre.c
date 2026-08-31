/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/engine_centre.c
 *
 * PURPOSE:
 *   Render Framework engine capability truth as a Studio view without copying
 *   engine definitions, implementation state or adapter policy into the IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/engine_centre.h"

#include <stdio.h>
#include <string.h>

/* Set one string property through the reusable Framework value contract. */
static UmiStatus set_string(
    UmiUiViewModel *view,
    const char *key,
    const char *text)
{
    UmiUiValue value;
    UmiStatus status;
    if (view == NULL || key == NULL || text == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_ui_value_set_string(&value, text);
    return status == UMI_STATUS_OK
        ? umi_ui_view_model_set_property(view, key, &value) : status;
}

/* Set one non-negative count after a checked conversion to the UI integer type. */
static UmiStatus set_count(
    UmiUiViewModel *view,
    const char *key,
    size_t count)
{
    UmiUiValue value;
    UmiStatus status;
    /* Counts beyond INT64_MAX cannot be represented by the common UI value. */
    if (count > (size_t)INT64_MAX) return UMI_STATUS_CAPACITY_EXCEEDED;
    status = umi_ui_value_set_integer(&value, (int64_t)count);
    return status == UMI_STATUS_OK
        ? umi_ui_view_model_set_property(view, key, &value) : status;
}

/* Count maturity directly from descriptors so Studio cannot drift from Framework. */
UmiStatus umi_studio_engine_centre_snapshot(
    UmiStudioEngineCentreSnapshot *out_snapshot)
{
    size_t index;
    if (out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->engine_count = umi_engine_catalogue_count();
    for (index = 0U; index < out_snapshot->engine_count; ++index) {
        const UmiEngineDescriptor *descriptor = umi_engine_catalogue_at(index);
        /* Every catalogue index below count must resolve to one descriptor. */
        if (descriptor == NULL) return UMI_STATUS_INTERNAL_ERROR;
        if (descriptor->maturity == UMI_ENGINE_MATURITY_CONTRACT) {
            out_snapshot->contract_count += 1U;
        } else if (descriptor->maturity == UMI_ENGINE_MATURITY_FOUNDATION) {
            out_snapshot->foundation_count += 1U;
        } else if (descriptor->maturity == UMI_ENGINE_MATURITY_OPERATIONAL) {
            out_snapshot->operational_count += 1U;
        } else {
            return UMI_STATUS_INTERNAL_ERROR;
        }
    }
    return UMI_STATUS_OK;
}

/* Delegate project compatibility decisions to the canonical Framework validator. */
UmiStatus umi_studio_engine_centre_validate_project(
    const UmiEngineRequirement *requirements,
    size_t requirement_count,
    UmiEngineRequirementReport *out_report)
{
    return umi_engine_catalogue_validate(
        requirements, requirement_count, out_report);
}

/* Build a movable Engine Explorer view from borrowed descriptors and copied text. */
UmiStatus umi_studio_engine_centre_view_create(
    const char *view_id,
    UmiUiViewModel **out_view)
{
    UmiStudioEngineCentreSnapshot snapshot;
    UmiStatus status;
    size_t index;
    if (view_id == NULL || view_id[0] == '\0' || out_view == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_view = NULL;
    status = umi_studio_engine_centre_snapshot(&snapshot);
    if (status != UMI_STATUS_OK) return status;
    status = umi_ui_view_model_create(
        view_id, "umicom.studio.engine-centre", UMI_UI_ROLE_PANE, out_view);
    if (status == UMI_STATUS_OK) status = set_string(
        *out_view, "title", "Engine Explorer");
    if (status == UMI_STATUS_OK) status = set_string(
        *out_view, "summary",
        "Reusable Framework engines and their truthful implementation maturity.");
    if (status == UMI_STATUS_OK) status = set_count(
        *out_view, "engine.count", snapshot.engine_count);
    if (status == UMI_STATUS_OK) status = set_count(
        *out_view, "engine.operational-count", snapshot.operational_count);
    if (status == UMI_STATUS_OK) status = set_count(
        *out_view, "engine.foundation-count", snapshot.foundation_count);

    for (index = 0U; status == UMI_STATUS_OK &&
         index < snapshot.engine_count; ++index) {
        const UmiEngineDescriptor *descriptor = umi_engine_catalogue_at(index);
        char key[64];
        char row[512];
        int key_written;
        int row_written;
        /* Fixed buffers reject truncation rather than emitting ambiguous rows. */
        key_written = snprintf(key, sizeof(key), "engine.row.%zu", index);
        row_written = descriptor != NULL
            ? snprintf(row, sizeof(row), "%s — %s — %s",
                descriptor->display_name,
                umi_engine_maturity_text(descriptor->maturity),
                descriptor->summary)
            : -1;
        if (key_written < 0 || (size_t)key_written >= sizeof(key) ||
            row_written < 0 || (size_t)row_written >= sizeof(row)) {
            status = UMI_STATUS_CAPACITY_EXCEEDED;
        } else {
            status = set_string(*out_view, key, row);
        }
    }
    /* Destroy a partially constructed view so callers never own invalid state. */
    if (status != UMI_STATUS_OK && *out_view != NULL) {
        umi_ui_view_model_destroy(*out_view);
        *out_view = NULL;
    }
    return status;
}
