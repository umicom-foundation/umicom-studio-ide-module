/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/codeguard_workflow.h
 *
 * PURPOSE:
 *   Bind shared background quality reviews to the current Studio workspace lifetime.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_CODEGUARD_WORKFLOW_H
#define UMICOM_STUDIO_CODEGUARD_WORKFLOW_H
#include "umicom/studio/services.h"
#include "umicom/codeguard/review.h"
#ifdef __cplusplus
extern "C" {
#endif

/* These operations belong to the service container's creating thread. The
 * service owns its Framework review and queue; callers never release either.
 * Begin reads saved source files through Framework, without executing project
 * code, so an open untrusted workspace is permitted. NULL profile uses defaults.
 * It copies the graph revision as the context token and the absolute root.
 * A previous pending job must retire before another can be accepted.
 * Success writes a request ID scoped to this root-bound review; use it together
 * with context_id. Rebinding to a different root starts a new review lifetime
 * and drops the old root's report. Failed admission leaves *out unchanged. */
UmiStatus UmiStudioCodeGuardBegin(UmiStudioServices *services,
    const UmiCodeGuardProfile *profile, uint64_t *out_request_id);

/* Poll never joins a worker. It retires completed work and suppresses findings
 * after any workspace revision/root change, including close/reopen. API failure
 * preserves *out; scan/publication failures are returned in snapshot.status.
 * Snapshot reads the last poll cache without publishing or cancelling work.
 * Both hide reports which no longer match the current workspace context. */
UmiStatus UmiStudioCodeGuardPoll(UmiStudioServices *services,
    UmiCodeGuardReviewSnapshot *out);
UmiStatus UmiStudioCodeGuardSnapshot(const UmiStudioServices *services,
    UmiCodeGuardReviewSnapshot *out);

/* May be used after closing the workspace. Cancellation is cooperative and
 * does not free running work. With no pending review this is an OK no-op. */
UmiStatus UmiStudioCodeGuardCancel(UmiStudioServices *services);

/* Borrow a report for the current workspace only. NULL also covers a wrong
 * thread, absent report or invalid owner. Finding pointers expire on a
 * successful replacement, root rebinding or service destruction. These findings
 * describe saved files, not unsaved editor buffers or a filesystem snapshot. */
const UmiCodeGuardResult *UmiStudioCodeGuardResult(const UmiStudioServices *services);

#ifdef __cplusplus
}
#endif
#endif
