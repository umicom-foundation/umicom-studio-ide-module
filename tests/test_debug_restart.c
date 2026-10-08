/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_debug_restart.c
 * PURPOSE: Verify Studio restart delegates to the active Framework adapter.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/debugger.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                                                             \
    do                                                                                                       \
    {                                                                                                        \
        if (!(x))                                                                                            \
        {                                                                                                    \
            fprintf(stderr, "%d: %s\n", __LINE__, #x);                                                       \
            result = 1;                                                                                      \
            goto done;                                                                                       \
        }                                                                                                    \
    } while (0)
#define OK(x) CHECK((x) == UMI_STATUS_OK)
int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;
    int result = 0;
    UmiStudioDebuggerService *service = NULL;
    OK(umi_studio_debugger_service_create(&service));
    CHECK(umi_studio_debugger_service_stop(service, 1) == UMI_STATUS_INVALID_STATE);
    UmiDebugRuntimePlatform *platform = UmiStudioDebuggerNativePlatform(service);
    UmiDebugService *model = umi_studio_debugger_service_model(service);
    UmiDebugAdapterProfile profile = *umi_debug_runtime_builtin_profile_find("debug.adapter.lldb-dap");
    CHECK(strlen(argv[1]) < sizeof profile.executable);
    strcpy(profile.executable, argv[1]);
    int length = snprintf(profile.arguments, sizeof profile.arguments, "session-restart-%s", argv[2]);
    CHECK(length > 0 && (size_t)length < sizeof profile.arguments);
    OK(umi_debug_adapter_profile_registry_upsert(umi_debug_service_adapter_profiles(model), &profile));
    OK(umi_debug_runtime_platform_start(platform, profile.id, "studio-restart", "launch", "{}", 0, NULL,
                                        2000U));
    UmiDebugRuntimePlatformSnapshot before = {0}, after;
    for (unsigned i = 0U; i < 100U; ++i)
    {
        OK(umi_debug_runtime_platform_snapshot(platform, &before));
        if (before.paused)
            break;
        int handled = 0;
        UmiStatus poll = umi_debug_runtime_platform_pump_event(platform, 25U, &handled);
        CHECK(poll == UMI_STATUS_OK || poll == UMI_STATUS_NOT_FOUND);
    }
    CHECK(before.paused);
    UmiStatus status = umi_studio_debugger_service_stop(service, 1);
    OK(umi_debug_runtime_platform_snapshot(platform, &after));
    if (strcmp(argv[2], "unsupported") == 0)
    {
        CHECK(status == UMI_STATUS_NOT_IMPLEMENTED && after.paused);
        CHECK(after.adapter.messages_sent == before.adapter.messages_sent);
    }
    else if (strcmp(argv[2], "rejected") == 0)
    {
        CHECK(status == UMI_STATUS_UNAVAILABLE && !after.paused);
        CHECK(umi_studio_debugger_service_stop(service, 1) == UMI_STATUS_INVALID_STATE);
    }
    else
    {
        CHECK(status == UMI_STATUS_OK && !after.paused);
        CHECK(after.adapter.messages_sent == before.adapter.messages_sent + 1U);
    }
    OK(umi_studio_debugger_service_stop(service, 0));
done:
    umi_studio_debugger_service_destroy(service);
    return result;
}
