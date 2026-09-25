/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_trading_service.c
 *
 * PURPOSE:
 *   Verify Studio owns a deterministic, simulation-first Framework trading
 *   workspace and never reports broker or live readiness by default.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "umicom/studio/trading.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioTradingService *service = NULL;
    UmiTradingWorkspace *workspace;
    UmiTradingWorkspaceSnapshot snapshot;
    UmiStrategyResearchWorkspaceSnapshot research;
    UmiStrategyProjectConfig strategyConfig;
    char strategySource[2048];
    size_t strategyRequired = 0U;
    UmiRiskDecision decision;

    assert(umi_studio_trading_service_create(&service) == UMI_STATUS_OK);
    workspace = umi_studio_trading_service_workspace(service);
    assert(workspace != NULL);
    assert(umi_studio_trading_service_snapshot(service, &snapshot) ==
           UMI_STATUS_OK);
    assert(strcmp(snapshot.account_id.value, "studio.simulation") == 0);
    assert(snapshot.environment == UMI_TRADING_SIMULATION);
    assert(snapshot.watchlist_count == 3U);
    assert(snapshot.visible_instrument_count == 3U);
    assert(snapshot.market_data_ready);
    assert(snapshot.trade_tape.provider_ready);
    assert(snapshot.trade_tape.retained_count == 3U);
    assert(snapshot.risk_ready);
    assert(!snapshot.broker_ready);
    assert(!snapshot.live_armed);
    assert(snapshot.can_preview_order);
    assert(snapshot.can_submit_order);

    /* Studio consumes Framework research readiness rather than owning another
     * backtest/replay state model. Before selection, the research service
     * correctly reports that no instrument-specific strategy is ready. */
    assert(umi_studio_trading_service_strategy_research_snapshot(
               service, &research) == UMI_STATUS_OK);
    assert(research.marketDataReady);
    assert(research.riskReady);
    assert(research.healthReady);
    assert(!research.strategyReady);

    assert(umi_trading_workspace_select_instrument(
               workspace, "CME.ES.REFERENCE") == UMI_STATUS_OK);
    assert(umi_studio_trading_service_strategy_research_snapshot(
               service, &research) == UMI_STATUS_OK);
    assert(research.strategyReady);
    assert(research.replayReady);
    assert(research.studiesReady);
    assert(research.simulationReady);
    assert(research.optimisationReady);

    umi_strategy_project_config_init(&strategyConfig);
    (void)snprintf(
        strategyConfig.projectName,
        sizeof(strategyConfig.projectName),
        "%s",
        "StudioStrategy");
    assert(umi_studio_trading_service_render_strategy_template(
               service,
               &strategyConfig,
               strategySource,
               sizeof(strategySource),
               &strategyRequired) == UMI_STATUS_OK);
    assert(strategyRequired > 0U);
    assert(strstr(strategySource, "strategy.signal-score") != NULL);
    assert(strstr(strategySource, "submit") == NULL);

    assert(umi_trading_workspace_set_draft_quantity(workspace, 1.0) ==
           UMI_STATUS_OK);
    assert(umi_trading_workspace_preview_order(workspace, &decision) ==
           UMI_STATUS_OK);
    assert(decision.allowed);

    assert(umi_trading_workspace_set_environment(
               workspace, UMI_TRADING_PAPER) == UMI_STATUS_OK);
    assert(umi_studio_trading_service_snapshot(service, &snapshot) ==
           UMI_STATUS_OK);
    assert(!snapshot.health_ready);
    assert(!snapshot.can_submit_order);
    assert(umi_trading_workspace_set_environment(
               workspace, UMI_TRADING_LIVE) == UMI_STATUS_OK);
    assert(umi_trading_workspace_set_live_armed(workspace, 1) ==
           UMI_STATUS_INVALID_STATE);

    umi_studio_trading_service_destroy(service);
    return 0;
}
