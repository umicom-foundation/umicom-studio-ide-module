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
#include "umicom/strategy_research/service.h"
#include "umicom/strategy_research/strategy_project.h"

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

/**
 * Copy Framework-owned strategy/backtest/replay readiness for Studio.
 *
 * Studio remains the coding and inspection frontend. Strategy research state,
 * replay rules, optimisation and simulation evidence remain Framework-owned
 * and cannot arm live execution through this API.
 */
UmiStatus umi_studio_trading_service_strategy_research_snapshot(
    UmiStudioTradingService *service,
    UmiStrategyResearchWorkspaceSnapshot *out_snapshot);


/**
 * Render a simulation-only C23 strategy source template through Framework.
 *
 * Studio may place the returned source in its normal project/document workflow;
 * the generated code produces research evidence and has no broker submit path.
 */
UmiStatus umi_studio_trading_service_render_strategy_template(
    UmiStudioTradingService *service,
    const UmiStrategyProjectConfig *config,
    char *out_source,
    size_t capacity,
    size_t *out_required);

#ifdef __cplusplus
}
#endif

#endif
