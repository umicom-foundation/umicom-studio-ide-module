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
#include "umicom/ui/workbench.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_PANE_DEBUG_REGISTERS "studio.pane.debug-registers"
#define UMI_STUDIO_PANE_DEBUG_DISASSEMBLY "studio.pane.debug-disassembly"
#define UMI_STUDIO_VIEW_DEBUG_REGISTERS "studio.debug-registers"
#define UMI_STUDIO_VIEW_DEBUG_DISASSEMBLY "studio.debug-disassembly"
#define UMI_STUDIO_DEBUG_LOW_LEVEL_REFRESH_COMMAND \
    "studio.debug.low-level.refresh"
#define UMI_STUDIO_DEBUG_LOW_LEVEL_REFRESH_ACTION \
    "studio.action.debug.low-level.refresh"
#define UMI_STUDIO_DEBUG_INSTRUCTION_NEXT_COMMAND \
    "studio.debug.instruction.next"
#define UMI_STUDIO_DEBUG_INSTRUCTION_NEXT_ACTION \
    "studio.action.debug.instruction.next"
#define UMI_STUDIO_DEBUG_INSTRUCTION_STEP_IN_COMMAND \
    "studio.debug.instruction.step-in"
#define UMI_STUDIO_DEBUG_INSTRUCTION_STEP_IN_ACTION \
    "studio.action.debug.instruction.step-in"
#define UMI_STUDIO_DEBUG_INSTRUCTION_BREAKPOINT_COMMAND \
    "studio.debug.instruction.breakpoint-toggle"
#define UMI_STUDIO_DEBUG_INSTRUCTION_BREAKPOINT_ACTION \
    "studio.action.debug.instruction.breakpoint-toggle"

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

/**
 * Initialise Studio's Registers pane from the Framework register-bank projection.
 */
UmiStatus umi_studio_debug_registers_view_create(
    const char *view_id,
    UmiStudioDebuggerService *debugger,
    UmiUiViewModel **out_view);

/**
 * Initialise Studio's Disassembly pane from the Framework disassembly projection.
 */
UmiStatus umi_studio_debug_disassembly_view_create(
    const char *view_id,
    UmiStudioDebuggerService *debugger,
    UmiUiViewModel **out_view);

/**
 * Register Studio's thin low-level debugging composition: two panes, their view factories,
 * one refresh command and the corresponding action.  Register/disassembly semantics remain
 * Framework-owned.
 */
UmiStatus umi_studio_debug_low_level_register(
    UmiUiWorkbench *workbench,
    UmiStudioDebuggerService *debugger);

#ifdef __cplusplus
}
#endif

#endif
