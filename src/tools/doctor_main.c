/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/tools/doctor_main.c
 *
 * PURPOSE:
 *   Provide the doctor main development utility for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/doctor_main.c
 *---------------------------------------------------------------------------*/
#include "umicom/studio/doctor.h"

#include <stdio.h>

/* Provide the console sink operation used by this module and its client applications. */
static void console_sink(const UmiDiagnostic *diagnostic, void *user_data)
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
int main(int argc, char **argv)
{
    const char *root = argc > 1 ? argv[1] : ".";
    UmiStudioDoctorReport report;
    UmiStatus status = umi_studio_doctor_run(root, console_sink, NULL, &report);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr, "Doctor failed to run: %s\n", umi_status_text(status));
        return 2;
    }
    (void)printf("Checks passed: %zu\n", report.checks_passed);
    (void)printf("Checks failed: %zu\n", report.checks_failed);
    return report.checks_failed == 0U ? 0 : 1;
}
