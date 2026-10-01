/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_breakpoint_properties_live.c
 * PURPOSE: Verify separate local-intent and live adapter outcomes through the real Studio facade.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/


#include "umicom/studio/debugger.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); failed = 1; goto done; } } while (0)
#define OK(x) CHECK((x) == UMI_STATUS_OK)
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    int failed = 0; UmiStudioDebuggerService *service = NULL; UmiDebugBreakpointEdit *edit = NULL;
    OK(umi_studio_debugger_service_create(&service));
    UmiDebugRuntimePlatform *platform = UmiStudioDebuggerNativePlatform(service);
    UmiDebugService *model = umi_studio_debugger_service_model(service);
    UmiDebugAdapterProfile profile = *umi_debug_runtime_builtin_profile_find("debug.adapter.lldb-dap");
    (void)snprintf(profile.executable, sizeof profile.executable, "%s", argv[1]);
    (void)snprintf(profile.arguments, sizeof profile.arguments, "properties-%s", argv[2]);
    OK(umi_debug_adapter_profile_registry_upsert(umi_debug_service_adapter_profiles(model), &profile));
    /* Controlled peer only: this verifies facade composition, not native
     * debugging or the separate saved-source/build-before-launch workflow. */
    OK(umi_debug_runtime_platform_start(platform, profile.id, "session", "launch", "{}", 0, NULL, 2000U));
    UmiDebugBreakpointSnapshot value = {0}; strcpy(value.id, "first"); strcpy(value.uri, "main.c");
    value.line = 13; value.column = 1; value.enabled = 1;
    OK(umi_debug_breakpoint_registry_upsert(umi_debug_service_breakpoint(model), &value));
    OK(UmiDebugBreakpointEditCapture(umi_studio_debugger_service_workspace(service), 0U, &edit));
    UmiDebugBreakpointSettings settings; OK(UmiDebugBreakpointSettingsInit(&settings, 1, "count > 3", "count={count}"));
    UmiStudioBreakpointEditResult result;
    UmiStatus status = UmiStudioDebuggerEditBreakpoint(service, edit, &settings, 0, &result);
    CHECK(result.desiredApplied && result.desiredChanged);
    OK(umi_debug_breakpoint_registry_find(umi_debug_service_breakpoint(model), "first", &value));
    CHECK(strcmp(value.condition, settings.condition) == 0 && strcmp(value.log_message, settings.logMessage) == 0);
    if (strcmp(argv[2], "success") == 0) CHECK(status == UMI_STATUS_OK && result.adapterSynchronized && value.verified && value.line == 15);
    else {
        CHECK(strcmp(argv[2], "rejected") == 0 || strcmp(argv[2], "unsupported") == 0);
        CHECK(status == UMI_STATUS_UNAVAILABLE && !result.adapterSynchronized && !value.verified && value.line == 13);
    }
    OK(umi_debug_runtime_platform_stop(platform, 1, 2000U));
done:
    UmiDebugBreakpointEditDestroy(edit); umi_studio_debugger_service_destroy(service); return failed;
}
