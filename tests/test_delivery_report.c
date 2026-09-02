/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_delivery_report.c
 *
 * PURPOSE:
 *   Verify one Studio integration path over the Framework delivery platform.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * The test keeps product integration small so failures are easy for a new developer to locate.
 */

#include <assert.h>
#include "umicom/studio/delivery_report.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void) {
    UmiReleaseCandidate candidate;
    UmiDeliveryPipeline pipeline;
    UmiDeliveryReport report;
    UmiReleaseGate gate;
    umi_release_candidate_init(&candidate);
    assert(umi_delivery_pipeline_init(&pipeline, "p") == UMI_STATUS_OK);
    assert(umi_release_gate_init(&gate, "g", 0) == UMI_STATUS_OK);
    assert(umi_delivery_pipeline_add_gate(&pipeline, &gate) == UMI_STATUS_OK);
    assert(umi_studio_delivery_report(&candidate, &pipeline, &report) == UMI_STATUS_OK);
    assert(report.gates == 1U);
    return 0;
}
