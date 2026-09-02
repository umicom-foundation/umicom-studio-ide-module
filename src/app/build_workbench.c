/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/build_workbench.c
 *
 * PURPOSE:
 *   Publish structured compiler/test diagnostics and raw output to the
 *   toolkit-neutral Studio workbench registries.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/build_workbench.h"

/*
 * Provide the studio build publish result operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_build_publish_result(
    const UmiBuildResult *result,
    UmiUiProblemRegistry *problems,
    UmiUiOutputChannelRegistry *output_channels)
{
    UmiDiagnosticPipeline *pipeline = NULL;
    UmiDiagnosticPipelineConfig config;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (result == NULL || problems == NULL || output_channels == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    config = umi_diagnostic_pipeline_config_default();
    config.mirror_diagnostics_to_output = 0;
    status = umi_diagnostic_pipeline_create(&config, &pipeline);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_diagnostic_build_result_ingest(pipeline, result, "Umicom Build");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_diagnostic_ui_sync_registries(pipeline, problems, output_channels);
    }
    umi_diagnostic_pipeline_destroy(pipeline);
    return status;
}
