/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_diagnostics_pipeline.c
 *
 * PURPOSE:
 *   Verify the test diagnostics pipeline behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Verify Studio composes the Framework-owned diagnostics and output pipeline. */
/* Setup and contract assertions remain active in optimized test builds. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "studio_test_state.h"
#include <assert.h>
#include <string.h>

#include "umicom/studio/diagnostics.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioServices *services = NULL;
    UmiBuildResult *result = NULL;
    UmiBuildDiagnostic diagnostic = {0};
    UmiDiagnosticPipelineSnapshot snapshot;
    /* Keep the former caller-relative cleanup for review. Each invocation now
     * owns a unique state directory, so direct execution cannot erase user state. */
#if 0
    (void)umi_fs_remove_tree(".umicom");
#endif
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
    assert(umi_build_result_create(&result) == UMI_STATUS_OK);
    assert(umi_studio_services_create(NULL, NULL, &services) == UMI_STATUS_OK);
    umi_build_result_init(result, 77U, UMI_BUILD_PHASE_BUILD, "debug");
    (void)strcpy(result->output, "src/main.c:8:4: error: invalid token\n");
    (void)strcpy(diagnostic.file, "src/main.c");
    (void)strcpy(diagnostic.message, "invalid token");
    diagnostic.line = 8U;
    diagnostic.column = 4U;
    diagnostic.severity = UMI_BUILD_DIAGNOSTIC_ERROR;
    assert(umi_build_diagnostic_list_add(&result->diagnostics, &diagnostic) == UMI_STATUS_OK);
    assert(umi_studio_diagnostics_ingest_build_result(services, result) == UMI_STATUS_OK);
    assert(umi_studio_diagnostics_pipeline_snapshot(services, &snapshot) == UMI_STATUS_OK);
    assert(snapshot.diagnostics.retained_count == 1U);
    assert(snapshot.output.retained_count >= 1U);
    assert(umi_studio_diagnostics_clear_output(services) == UMI_STATUS_OK);
    assert(umi_studio_diagnostics_pipeline_snapshot(services, &snapshot) == UMI_STATUS_OK);
    assert(snapshot.output.retained_count == 0U);
    umi_studio_services_destroy(services);
    umi_build_result_destroy(result);
    /* Retain generated evidence instead of deleting a launch-directory tree. */
#if 0
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
#endif
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
