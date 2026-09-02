/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/diagnostics.h
 *
 * PURPOSE:
 *   Provide Studio-facing operations for emitting, summarising, reading, and
 *   clearing retained Framework diagnostics without exposing the diagnostic
 *   hub implementation to every Studio feature.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DIAGNOSTICS_H
#define UMICOM_STUDIO_DIAGNOSTICS_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio diagnostics emit operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_diagnostics_emit(
    UmiStudioServices *services,
    UmiDiagnosticSeverity severity,
    const char *source,
    const char *message,
    uint64_t correlation_id
);

/**
 * Provide the studio diagnostics summary operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_diagnostics_summary(
    UmiStudioServices *services,
    UmiDiagnosticStoreSummary *out_summary
);

/**
 * Return the number of records represented by studio diagnostics without changing their
 * state.
 */
size_t umi_studio_diagnostics_count(UmiStudioServices *services);

/**
 * Find studio diagnostics record while leaving the underlying catalogue or model owned by
 * this module.
 */
UmiStatus umi_studio_diagnostics_record_at(
    UmiStudioServices *services,
    size_t chronological_index,
    UmiDiagnosticRecord *out_record
);

/**
 * Release or reset state held by studio diagnostics so the same storage can be reused
 * safely.
 */
UmiStatus umi_studio_diagnostics_clear(UmiStudioServices *services);
/**
 * Provide the studio diagnostics clear problems operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_diagnostics_clear_problems(UmiStudioServices *services);
/**
 * Provide the studio diagnostics clear output operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_diagnostics_clear_output(UmiStudioServices *services);
/**
 * Provide the studio diagnostics pipeline snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_diagnostics_pipeline_snapshot(
    UmiStudioServices *services,
    UmiDiagnosticPipelineSnapshot *out_snapshot
);
/**
 * Provide the studio diagnostics ingest build result operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_diagnostics_ingest_build_result(
    UmiStudioServices *services,
    const UmiBuildResult *result
);

#ifdef __cplusplus
}
#endif

#endif
