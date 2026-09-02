/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/debug_workspace_views.h
 *
 * PURPOSE:
 *   Compose professional Studio Debug panes from the reusable Framework
 *   debugger-workspace model without introducing frontend toolkit ownership.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_DEBUG_WORKSPACE_VIEWS_H
#define UMICOM_STUDIO_DEBUG_WORKSPACE_VIEWS_H

#include "umicom/studio/debugger.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialise studio debug call stack view from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_debug_call_stack_view_create(
    const char *view_id,
    UmiStudioDebuggerService *debugger,
    UmiUiViewModel **out_view
);
/**
 * Initialise studio debug variables view from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_debug_variables_view_create(
    const char *view_id,
    UmiStudioDebuggerService *debugger,
    UmiUiViewModel **out_view
);
/**
 * Initialise studio debug watches view from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_debug_watches_view_create(
    const char *view_id,
    UmiStudioDebuggerService *debugger,
    UmiUiViewModel **out_view
);
/**
 * Initialise studio debug breakpoints view from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_debug_breakpoints_view_create(
    const char *view_id,
    UmiStudioDebuggerService *debugger,
    UmiUiViewModel **out_view
);
/**
 * Initialise studio debug console view from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_debug_console_view_create(
    const char *view_id,
    UmiStudioDebuggerService *debugger,
    UmiUiViewModel **out_view
);

#ifdef __cplusplus
}
#endif

#endif
