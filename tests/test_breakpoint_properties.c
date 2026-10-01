/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_breakpoint_properties.c
 * PURPOSE: Check thin product editing and truthful deferred adapter status.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/debugger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); exit(1); } } while (0)
#define OK(x) CHECK((x) == UMI_STATUS_OK)
int main(int argc, char **argv)
{
    CHECK(argc == 2); const char *name = argv[1]; UmiStudioDebuggerService *service = NULL;
    OK(umi_studio_debugger_service_create(&service));
    OK(umi_studio_debugger_service_add_breakpoint(service, "main.c", 13, 1));
    UmiDebugWorkspace *workspace = umi_studio_debugger_service_workspace(service);
    UmiDebugBreakpointEdit *edit = NULL; OK(UmiDebugBreakpointEditCapture(workspace, 0, &edit));
    UmiDebugBreakpointSettings settings; OK(UmiDebugBreakpointSettingsInit(&settings, 1, "count > 3", "count={count}"));
    UmiStudioBreakpointEditResult result; UmiDebugBreakpointSnapshot actual;
    if (strcmp(name, "stale") == 0) {
        OK(umi_studio_debugger_service_add_breakpoint(service, "other.c", 4, 1));
        CHECK(UmiStudioDebuggerEditBreakpoint(service, edit, &settings, 0, &result) == UMI_STATUS_BUSY && !result.desiredApplied);
    } else {
        if (strcmp(name, "memory") == 0) OK(umi_studio_debugger_service_initialize(service, "memory", NULL));
        else CHECK(strcmp(name, "idle") == 0 || strcmp(name, "remove") == 0);
        OK(UmiStudioDebuggerEditBreakpoint(service, edit, &settings, 0, &result));
        CHECK(result.desiredApplied && result.desiredChanged && !result.adapterSynchronized);
        OK(umi_debug_workspace_breakpoint_at(workspace, 0, &actual));
        CHECK(strcmp(actual.condition, settings.condition) == 0 && strcmp(actual.log_message, settings.logMessage) == 0 && !actual.verified);
        if (strcmp(name, "remove") == 0) {
            UmiDebugBreakpointEditDestroy(edit); edit = NULL;
            OK(UmiDebugBreakpointEditCapture(workspace, 0, &edit));
            OK(UmiStudioDebuggerEditBreakpoint(service, edit, NULL, 1, &result));
            CHECK(result.desiredApplied && result.desiredChanged && umi_debug_workspace_breakpoint_at(workspace, 0, &actual) == UMI_STATUS_NOT_FOUND);
        }
    }
    CHECK(!UmiStudioDebuggerNativeBusy(service));
    UmiDebugBreakpointEditDestroy(edit); umi_studio_debugger_service_destroy(service); return 0;
}
