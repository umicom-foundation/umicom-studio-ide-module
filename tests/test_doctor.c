/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_doctor.c
 *
 * PURPOSE:
 *   Verify that the native Studio Doctor can inspect the repository and report
 *   a healthy Framework-based Studio source tree with no missing required
 *   files and no active legacy migration paths or CMake identifiers.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/doctor.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * Exercise test console sink and return a clear result when the behaviour no longer
 * matches its contract.
 */
static void test_console_sink(const UmiDiagnostic *diagnostic, void *user_data)
{
    (void)user_data;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (diagnostic == NULL) {
        return;
    }

    (void)printf("[%s] %s\n",
                 umi_diagnostic_severity_text(diagnostic->severity),
                 diagnostic->message != NULL ? diagnostic->message : "");
}

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioDoctorReport report = {0U, 0U};
    UmiStatus status;

    status = umi_studio_doctor_run(UMICOM_STUDIO_SOURCE_ROOT,
                                   test_console_sink,
                                   NULL,
                                   &report);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr,
                      "Studio Doctor test could not run: %s\n",
                      umi_status_text(status));
        return EXIT_FAILURE;
    }

    (void)printf("Checks passed: %zu\n", report.checks_passed);
    (void)printf("Checks failed: %zu\n", report.checks_failed);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (report.checks_failed != 0U) {
        (void)fprintf(stderr,
                      "Studio Doctor found %zu failed repository check(s).\n",
                      report.checks_failed);
        return EXIT_FAILURE;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (report.checks_passed < 10U) {
        (void)fprintf(stderr,
                      "Studio Doctor completed too few checks: %zu.\n",
                      report.checks_passed);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
