/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/diagnostics.c
 *
 * PURPOSE:
 *   Implement the Studio-facing diagnostic API as a narrow adapter over the
 *   Framework diagnostic hub and bounded diagnostic store owned by
 *   UmiStudioServices.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/diagnostics.h"

/*
 * Provide the studio diagnostics emit operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_diagnostics_emit(
    UmiStudioServices *services,
    UmiDiagnosticSeverity severity,
    const char *source,
    const char *message,
    uint64_t correlation_id)
{
    UmiDiagnosticSink sink;
    void *user_data;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    sink = umi_studio_services_diagnostic_sink();
    user_data = umi_studio_services_diagnostic_user_data(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (sink == NULL || user_data == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }

    umi_diagnostic_emit(sink,
                        user_data,
                        severity,
                        source,
                        message,
                        correlation_id);
    return UMI_STATUS_OK;
}

/*
 * Provide the studio diagnostics summary operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_diagnostics_summary(
    UmiStudioServices *services,
    UmiDiagnosticStoreSummary *out_summary)
{
    UmiDiagnosticStore *store;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || out_summary == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    store = umi_studio_services_diagnostic_store(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (store == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }

    return umi_diagnostic_store_summary(store, out_summary);
}

/*
 * Return the number of records represented by studio diagnostics without changing their
 * state.
 */
size_t umi_studio_diagnostics_count(UmiStudioServices *services)
{
    UmiDiagnosticStore *store =
        umi_studio_services_diagnostic_store(services);
    return store != NULL ? umi_diagnostic_store_count(store) : 0U;
}

/*
 * Find studio diagnostics record while leaving the underlying catalogue or model owned by
 * this module.
 */
UmiStatus umi_studio_diagnostics_record_at(
    UmiStudioServices *services,
    size_t chronological_index,
    UmiDiagnosticRecord *out_record)
{
    UmiDiagnosticStore *store;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || out_record == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    store = umi_studio_services_diagnostic_store(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (store == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }

    return umi_diagnostic_store_record_at(store,
                                          chronological_index,
                                          out_record);
}

/*
 * Release or reset state held by studio diagnostics so the same storage can be reused
 * safely.
 */
UmiStatus umi_studio_diagnostics_clear(UmiStudioServices *services)
{
    UmiDiagnosticStore *store;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    store = umi_studio_services_diagnostic_store(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (store == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }

    umi_diagnostic_store_clear(store);
    umi_diagnostic_pipeline_clear(
        umi_studio_services_diagnostic_pipeline(services));
    return UMI_STATUS_OK;
}

/*
 * Provide the studio diagnostics clear problems operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_diagnostics_clear_problems(UmiStudioServices *services)
{
    UmiDiagnosticPipeline *pipeline;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    pipeline = umi_studio_services_diagnostic_pipeline(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (pipeline == NULL) return UMI_STATUS_INVALID_STATE;
    umi_diagnostic_store_clear(umi_studio_services_diagnostic_store(services));
    umi_diagnostic_model_clear(umi_diagnostic_pipeline_model(pipeline));
    return UMI_STATUS_OK;
}

/*
 * Provide the studio diagnostics clear output operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_diagnostics_clear_output(UmiStudioServices *services)
{
    UmiDiagnosticPipeline *pipeline;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    pipeline = umi_studio_services_diagnostic_pipeline(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (pipeline == NULL) return UMI_STATUS_INVALID_STATE;
    umi_output_buffer_clear(umi_diagnostic_pipeline_output(pipeline));
    return UMI_STATUS_OK;
}

/*
 * Provide the studio diagnostics pipeline snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_diagnostics_pipeline_snapshot(
    UmiStudioServices *services,
    UmiDiagnosticPipelineSnapshot *out_snapshot)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_diagnostic_pipeline_snapshot(
        umi_studio_services_diagnostic_pipeline(services), out_snapshot);
}

/*
 * Provide the studio diagnostics ingest build result operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_diagnostics_ingest_build_result(
    UmiStudioServices *services,
    const UmiBuildResult *result)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || result == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_diagnostic_build_result_ingest(
        umi_studio_services_diagnostic_pipeline(services), result, "Umicom Build");
}
