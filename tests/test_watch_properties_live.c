/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_watch_properties_live.c
 * PURPOSE: Verify explicit product evaluation delegates to shared paused-frame checks.
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
    CHECK(argc == 3); UmiStudioDebuggerService *service = NULL;
    OK(umi_studio_debugger_service_create(&service));
    UmiDebugRuntimePlatform *platform = UmiStudioDebuggerNativePlatform(service);
    UmiDebugService *model = umi_studio_debugger_service_model(service);
    UmiDebugWorkspace *workspace = umi_studio_debugger_service_workspace(service);
    UmiDebugAdapterProfile profile = *umi_debug_runtime_builtin_profile_find("debug.adapter.lldb-dap");
    (void)snprintf(profile.executable, sizeof profile.executable, "%s", argv[1]);
    (void)snprintf(profile.arguments, sizeof profile.arguments, "watch-edit-%s", argv[2]);
    OK(umi_debug_adapter_profile_registry_upsert(umi_debug_service_adapter_profiles(model), &profile));
    char id[128]; OK(umi_studio_debugger_service_add_watch(service, "savedNotes", id, sizeof id));
    OK(umi_debug_runtime_platform_start(platform, profile.id, "session", "launch", "{}", 0, NULL, 2000U));
    UmiDebugRuntimePlatformSnapshot state;
    for (unsigned i = 0U; i < 40U; ++i) {
        OK(umi_debug_runtime_platform_snapshot(platform, &state)); if (state.paused) break;
        int handled = 0; UmiStatus status = umi_debug_runtime_platform_pump_event(platform, 25U, &handled);
        CHECK(status == UMI_STATUS_OK || status == UMI_STATUS_NOT_FOUND);
    }
    OK(umi_debug_runtime_platform_snapshot(platform, &state));
    CHECK(state.paused); OK(UmiDebugRuntimePlatformInspectStopped(platform, 2000U));
    UmiDebugWatchEdit *edit = NULL; OK(UmiDebugWatchEditCapture(workspace, 0U, &edit));
    UmiDebugWatchSnapshot before, after; OK(umi_debug_workspace_watch_at(workspace, 0, &before));
    UmiStatus status = UmiStudioDebuggerEvaluateWatch(service, edit);
    OK(umi_debug_workspace_watch_at(workspace, 0, &after));
    if (strcmp(argv[2], "success") == 0) CHECK(status == UMI_STATUS_OK && after.valid && strcmp(after.value, "2") == 0);
    else { CHECK(strcmp(argv[2], "rejected") == 0); CHECK(status == UMI_STATUS_UNAVAILABLE && after.revision == before.revision); }
    OK(umi_debug_runtime_platform_stop(platform, 1, 2000U));
    UmiDebugWatchEditDestroy(edit); umi_studio_debugger_service_destroy(service); return 0;
}
