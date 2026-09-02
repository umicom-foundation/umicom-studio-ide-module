/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_production_readiness.c
 *
 * PURPOSE:
 *   Verify the test production readiness behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE production control-plane test | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/test_runtime/check.h"
#include <stdlib.h>
#include "umicom/studio/production_readiness.h"
/*
 * Exercise available and return a clear result when the behaviour no longer matches its
 * contract.
 */
static int available(const char *id, void *context) {
    (void)context; return id != NULL && id[0] != '\0';
}
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void) {
    UmiApplicationProductionRuntime *runtime = calloc(1U, sizeof(*runtime));
    UmiStudioProductionReadiness readiness;
    UMI_TEST_REQUIRE(runtime != NULL);
    UMI_TEST_REQUIRE(umi_application_production_runtime_init(umi_studio_productisation_contribution(), available, NULL, runtime) == UMI_STATUS_OK);
    UMI_TEST_REQUIRE(umi_studio_production_readiness_build(runtime, &readiness) == UMI_STATUS_OK);
    UMI_TEST_REQUIRE(readiness.launchable);
    UMI_TEST_REQUIRE(readiness.panel_count > 0U);
    free(runtime); return 0;
}

