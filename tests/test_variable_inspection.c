/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_variable_inspection.c
 * PURPOSE: Verify the product service delegates variable inspection without launching idle sessions.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/


#include "umicom/studio/debugger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%d: %s\n", __LINE__, #x); exit(1); } } while (0)
#define OK(x) CHECK((x) == UMI_STATUS_OK)
int main(int argc, char **argv)
{
    CHECK(argc == 3); const char *name = argv[2]; UmiStudioDebuggerService *service = NULL;
    OK(umi_studio_debugger_service_create(&service));
    UmiDebugService *model = umi_studio_debugger_service_model(service);
    UmiDebugWorkspace *workspace = umi_studio_debugger_service_workspace(service);
    UmiDebugRuntimePlatform *platform = UmiStudioDebuggerNativePlatform(service);
    UmiDebugVariableTarget *target = NULL; UmiDebugVariablePage *page = NULL;
    if (strcmp(name, "idle") == 0 || strcmp(name, "memory") == 0) {
        if (strcmp(name, "memory") == 0) OK(umi_studio_debugger_service_initialize(service, "memory", NULL));
        UmiDebugThreadSnapshot thread = {0}; strcpy(thread.id, "thread"); thread.stopped = 1;
        OK(umi_debug_thread_registry_upsert(umi_debug_service_thread(model), &thread));
        UmiDebugStackFrameSnapshot frame = {0}; strcpy(frame.id, "0"); strcpy(frame.thread_id, "thread");
        OK(umi_debug_stack_frame_registry_upsert(umi_debug_service_stack_frame(model), &frame));
        UmiDebugScopeSnapshot scope = {0}; strcpy(scope.id, "scope"); strcpy(scope.frame_id, "0");
        OK(umi_debug_scope_registry_upsert(umi_debug_service_scope(model), &scope));
        UmiDebugVariableSnapshot root = {0}; strcpy(root.id, "root"); strcpy(root.scope_id, "scope"); root.variables_reference = 2U;
        OK(umi_debug_variable_registry_upsert(umi_debug_service_variable(model), &root));
        OK(UmiDebugVariableTargetCapture(workspace, 0U, &target));
        CHECK(UmiStudioDebuggerInspectVariable(service, target, &page) == UMI_STATUS_INVALID_STATE && page == NULL);
        CHECK(!UmiStudioDebuggerNativeBusy(service));
    } else {
        UmiDebugAdapterProfile profile = *umi_debug_runtime_builtin_profile_find("debug.adapter.lldb-dap");
        (void)snprintf(profile.executable, sizeof profile.executable, "%s", argv[1]);
        (void)snprintf(profile.arguments, sizeof profile.arguments, "variable-inspection-%s", name);
        OK(umi_debug_adapter_profile_registry_upsert(umi_debug_service_adapter_profiles(model), &profile));
        OK(umi_debug_runtime_platform_start(platform, profile.id, "session", "launch", "{}", 0, NULL, 2000U));
        UmiDebugRuntimePlatformSnapshot state;
        for (unsigned i = 0U; i < 80U; ++i) {
            OK(umi_debug_runtime_platform_snapshot(platform, &state)); if (state.paused) break;
            int handled = 0; UmiStatus status = umi_debug_runtime_platform_pump_event(platform, 25U, &handled);
            CHECK(status == UMI_STATUS_OK || status == UMI_STATUS_NOT_FOUND);
        }
        CHECK(state.paused); OK(UmiDebugRuntimePlatformInspectStopped(platform, 2000U));
        OK(UmiDebugVariableTargetCapture(workspace, 0U, &target));
        UmiStatus status = UmiStudioDebuggerInspectVariable(service, target, &page);
        if (strcmp(name, "success") == 0) CHECK(status == UMI_STATUS_OK && UmiDebugVariablePageCount(page) == 2U);
        else { CHECK(strcmp(name, "rejected") == 0); CHECK(status == UMI_STATUS_UNAVAILABLE && page == NULL); }
        OK(umi_debug_runtime_platform_stop(platform, 1, 2000U));
    }
    UmiDebugVariableTargetDestroy(target); UmiDebugVariablePageDestroy(page);
    umi_studio_debugger_service_destroy(service); return 0;
}
