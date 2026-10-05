/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_debug_setup.c
 * PURPOSE: Verify Studio applies saved settings without starting or evaluating a debugger.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/debugger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                                                             \
    do                                                                                                       \
    {                                                                                                        \
        if (!(x))                                                                                            \
        {                                                                                                    \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);                                          \
            exit(1);                                                                                         \
        }                                                                                                    \
    } while (0)
#define OK(x) CHECK((x) == UMI_STATUS_OK)
int main(int argc, char **argv)
{
    CHECK(argc == 2);
    const char *mode = argv[1];
    UmiStudioDebuggerService *service = NULL;
    OK(umi_studio_debugger_service_create(&service));
    OK(umi_studio_debugger_service_add_breakpoint(service, "C:/source/old.c", 8, 0));
    UmiDebugSetup *setup = NULL;
    OK(UmiDebugSetupCreate("New settings", &setup));
    UmiDebugSetupBreakpoint point = {0};
    strcpy(point.source, "C:/source/new.c");
    point.line = 42U;
    point.enabled = 1;
    strcpy(point.condition, "count>1");
    OK(UmiDebugSetupAddBreakpoint(setup, &point));
    UmiDebugSetupWatch watch = {0};
    strcpy(watch.expression, "count");
    watch.enabled = 1;
    OK(UmiDebugSetupAddWatch(setup, &watch));
    if (strcmp(mode, "legacy-collision") == 0)
    {
        point.column = 9U;
        OK(UmiDebugSetupAddBreakpoint(setup, &point));
    }
    UmiDebugWorkspace *workspace = umi_studio_debugger_service_workspace(service);
    UmiDebugSetupReview *review = NULL;
    OK(UmiDebugSetupReviewCreate(workspace, setup, &review));
    if (strcmp(mode, "stale") == 0)
        OK(umi_debug_workspace_add_watch(workspace, "newer", NULL, 0U));
    if (strcmp(mode, "active") == 0)
        OK(umi_debug_controller_initialize(umi_studio_debugger_service_controller(service), "fixture"));
    UmiProtocolTransportStats before =
        umi_protocol_transport_stats(umi_studio_debugger_service_transport(service));
    UmiStatus status = UmiStudioDebuggerApplySetup(service, review);
    UmiDebugBreakpointSnapshot stored;
    OK(umi_debug_workspace_breakpoint_at(workspace, 0U, &stored));
    if (strcmp(mode, "apply") == 0)
    {
        CHECK(status == UMI_STATUS_OK && stored.line == 42U && strcmp(stored.condition, "count>1") == 0);
        CHECK(UmiStudioDebuggerApplySetup(service, review) == UMI_STATUS_BUSY);
        /* Legacy remove remains usable after restoration, and the canonical
         * desired collection must remove the same restored record. */
        OK(umi_studio_debugger_service_remove_breakpoint(service, stored.id));
        UmiDebugWorkspaceSnapshot snapshot;
        OK(umi_debug_workspace_snapshot(workspace, &snapshot));
        CHECK(snapshot.breakpoint_count == 0U && snapshot.watch_count == 1U);
    }
    else if (strcmp(mode, "stale") == 0 || strcmp(mode, "active") == 0 ||
             strcmp(mode, "legacy-collision") == 0)
    {
        CHECK(status != UMI_STATUS_OK && stored.line == 8U);
    }
    else
        return 2;
    UmiProtocolTransportStats after =
        umi_protocol_transport_stats(umi_studio_debugger_service_transport(service));
    CHECK(before.queued == after.queued && before.sent == after.sent && before.received == after.received);
    CHECK(!UmiStudioDebuggerNativeBusy(service));
    UmiDebugSetupReviewDestroy(review);
    UmiDebugSetupDestroy(setup);
    umi_studio_debugger_service_destroy(service);
    return 0;
}
