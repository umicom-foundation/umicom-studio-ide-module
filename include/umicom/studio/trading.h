/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/trading.h
 *
 * PURPOSE:
 *   Compose Studio's safe reference trading workspace from reusable Framework
 *   services while keeping broker connectivity and live execution outside the
 *   IDE product boundary.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio owns only the lifetime and deterministic demonstration market. The
 * trading rules, risk checks and view projections remain Framework-owned so a
 * dedicated Trader or TMS application can reuse them unchanged.
 */
#ifndef UMICOM_STUDIO_TRADING_H
#define UMICOM_STUDIO_TRADING_H

#include "umicom/trading/workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio trading service data shared with callers of this public contract.
 */
typedef struct UmiStudioTradingService UmiStudioTradingService;

/**
 * Initialise studio trading service from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_trading_service_create(
    UmiStudioTradingService **out_service);
/**
 * Release or reset state held by studio trading service so the same storage can be reused
 * safely.
 */
void umi_studio_trading_service_destroy(UmiStudioTradingService *service);
/**
 * Provide the studio trading service workspace operation used by this module and its
 * client applications.
 */
UmiTradingWorkspace *umi_studio_trading_service_workspace(
    UmiStudioTradingService *service);
/**
 * Provide the studio trading service snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_trading_service_snapshot(
    UmiStudioTradingService *service,
    UmiTradingWorkspaceSnapshot *out_snapshot);

#ifdef __cplusplus
}
#endif

#endif
