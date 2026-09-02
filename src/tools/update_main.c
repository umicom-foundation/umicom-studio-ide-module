/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/update_main.c
 *
 * PURPOSE:
 *   Provide a native Studio command for the Framework-backed application-delivery platform.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This command is a thin product-facing wrapper; the reusable delivery logic lives in Umicom Framework.
 */

#include <stdio.h>
#include "umicom/studio/delivery_update.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void) {
    UmiUpdatePlan plan;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_studio_update_plan(10U, 11U, 1, 1, &plan) != UMI_STATUS_OK) return 1;
    printf("Update: %s\n", umi_update_decision_text(plan.decision));
    printf("Target generation: %llu\n", (unsigned long long)plan.target_generation);
    return 0;
}
