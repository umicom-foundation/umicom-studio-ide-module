/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/debug_navigation.h
 *
 * PURPOSE:
 *   Bind Studio's selected native debugger to Framework-owned source/frame/
 *   disassembly navigation without duplicating debugger state in the product.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DEBUG_NAVIGATION_H
#define UMICOM_STUDIO_DEBUG_NAVIGATION_H

#include "umicom/debug/workbench/debug_navigation.h"
#include "umicom/studio/debugger.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef UmiDebugWorkbenchNavigationSnapshot UmiStudioDebugNavigationSnapshot;

static inline UmiStatus
umi_studio_debug_navigation_snapshot(
    UmiStudioDebuggerService *debugger,
    UmiStudioDebugNavigationSnapshot *out_snapshot)
{
    UmiDebugRuntimePlatform *platform;

    if (debugger == NULL || out_snapshot == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    platform = UmiStudioDebuggerNativePlatform(debugger);
    return platform != NULL
        ? umi_debug_workbench_navigation_snapshot(platform, out_snapshot)
        : UMI_STATUS_UNAVAILABLE;
}

static inline UmiStatus
umi_studio_debug_navigation_select_instruction(
    UmiStudioDebuggerService *debugger,
    const char *instruction_id,
    UmiStudioDebugNavigationSnapshot *out_snapshot)
{
    UmiDebugRuntimePlatform *platform;

    if (debugger == NULL || instruction_id == NULL ||
        out_snapshot == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    platform = UmiStudioDebuggerNativePlatform(debugger);
    return platform != NULL
        ? umi_debug_workbench_navigation_select_instruction(
              platform, instruction_id, out_snapshot)
        : UMI_STATUS_UNAVAILABLE;
}

static inline UmiStatus
umi_studio_debug_navigation_follow_source(
    UmiStudioDebuggerService *debugger,
    const char *source_uri,
    uint32_t source_line,
    UmiStudioDebugNavigationSnapshot *out_snapshot)
{
    UmiDebugRuntimePlatform *platform;

    if (debugger == NULL || source_uri == NULL ||
        out_snapshot == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    platform = UmiStudioDebuggerNativePlatform(debugger);
    return platform != NULL
        ? umi_debug_workbench_navigation_follow_source(
              platform, source_uri, source_line, out_snapshot)
        : UMI_STATUS_UNAVAILABLE;
}

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_DEBUG_NAVIGATION_H */
