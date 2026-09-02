/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: tests/test_productisation_contribution.c
 *
 * PURPOSE:
 *   Verify this module remains a thin, runnable and testable composition of
 *   canonical Framework product surfaces.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/test_runtime/check.h"
#include <stdlib.h>
#include <string.h>

#include "umicom/studio/productisation_contribution.h"
#include "umicom/application/productisation/workspace_guide_portfolio.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    const UmiProductApplicationAdoption *adoption =
        umi_studio_productisation_contribution();
    UmiProductApplicationAdoptionSnapshot *snapshot =
        (UmiProductApplicationAdoptionSnapshot *)calloc(1U, sizeof(*snapshot));
    UmiProductApplicationSession *session =
        (UmiProductApplicationSession *)calloc(1U, sizeof(*session));
    UmiProductApplicationSessionSnapshot *session_snapshot =
        (UmiProductApplicationSessionSnapshot *)calloc(
            1U, sizeof(*session_snapshot));
    UmiProductWorkspaceGuide *workspace_guide =
        (UmiProductWorkspaceGuide *)calloc(1U, sizeof(*workspace_guide));
    const UmiProductWorkspaceGuideChoice *recommended_workspace;
    UmiProductAdoptionRegistry adoption_registry;
    UmiProductWorkspaceGuidePortfolio *guide_portfolio =
        (UmiProductWorkspaceGuidePortfolio *)calloc(
            1U, sizeof(*guide_portfolio));
    const UmiProductWorkspaceGuideSummary *portfolio_summary;
    UmiProductApplicationSessionCommand command = {
        sizeof(UmiProductApplicationSessionCommand),
        UMI_PRODUCT_SESSION_REFRESH_READINESS,
        NULL,
        NULL,
        false
    };
    /* Product sessions and workspace portfolios contain bounded stores that
     * expand with the Framework catalogue, so test them in checked heap memory. */
    UMI_TEST_REQUIRE(adoption != NULL && snapshot != NULL && session != NULL &&
                     session_snapshot != NULL && workspace_guide != NULL &&
                     guide_portfolio != NULL);
    UMI_TEST_REQUIRE(strcmp(adoption->application_id, "org.umicom.studio") == 0);
    UMI_TEST_REQUIRE(umi_product_application_adoption_validate(adoption) ==
           UMI_STATUS_OK);
    UMI_TEST_REQUIRE(umi_studio_productisation_snapshot(snapshot) ==
           UMI_STATUS_OK);
    UMI_TEST_REQUIRE(snapshot->canonical_experience_available);
    UMI_TEST_REQUIRE(snapshot->feature_count > 0U);
    UMI_TEST_REQUIRE(snapshot->panel_count > 0U);
    UMI_TEST_REQUIRE(snapshot->layout_count > 0U);
    UMI_TEST_REQUIRE(snapshot->projected_layout_count == snapshot->layout_count);
    UMI_TEST_REQUIRE(snapshot->projected_window_count >= snapshot->panel_count);
    UMI_TEST_REQUIRE(snapshot->default_layout_window_count > 0U);
    UMI_TEST_REQUIRE(snapshot->layout_runtime_ready);
    UMI_TEST_REQUIRE(snapshot->layout_projection_complete);
    UMI_TEST_REQUIRE(snapshot->missing_surface_count == 0U);
    UMI_TEST_REQUIRE(snapshot->covered_surface_count == snapshot->panel_count);
    UMI_TEST_REQUIRE(snapshot->runnable);
    UMI_TEST_REQUIRE(snapshot->acceptance_ready);
    UMI_TEST_REQUIRE(umi_studio_product_session_init(session) ==
           UMI_STATUS_OK);
    UMI_TEST_REQUIRE(umi_product_application_session_execute(
        session, &command) == UMI_STATUS_OK);
    UMI_TEST_REQUIRE(umi_product_application_session_snapshot(
        session, session_snapshot) == UMI_STATUS_OK);
    UMI_TEST_REQUIRE(strcmp(session_snapshot->application_id,
                  adoption->application_id) == 0);
    UMI_TEST_REQUIRE(session_snapshot->command_count == 1U);
    UMI_TEST_REQUIRE(session_snapshot->successful_command_count == 1U);
    UMI_TEST_REQUIRE(session_snapshot->failed_command_count == 0U);
    UMI_TEST_REQUIRE(session_snapshot->readiness_percent <= 100U);
    UMI_TEST_REQUIRE(session_snapshot->runnable);
    UMI_TEST_REQUIRE(session_snapshot->acceptance_ready);
    UMI_TEST_REQUIRE(umi_studio_product_workspace_guide(workspace_guide) ==
           UMI_STATUS_OK);
    UMI_TEST_REQUIRE(umi_product_workspace_guide_validate(workspace_guide) ==
           UMI_STATUS_OK);
    UMI_TEST_REQUIRE(workspace_guide->choice_count == snapshot->layout_count);
    UMI_TEST_REQUIRE(workspace_guide->total_panel_placements ==
           snapshot->projected_window_count);
    UMI_TEST_REQUIRE(workspace_guide->readiness_percent <= 100U);
    recommended_workspace =
        umi_product_workspace_guide_recommended(workspace_guide);
    UMI_TEST_REQUIRE(recommended_workspace != NULL);
    UMI_TEST_REQUIRE(recommended_workspace->default_layout);
    UMI_TEST_REQUIRE(recommended_workspace->panel_count ==
           snapshot->default_layout_window_count);
    /* Prove this thin product can participate in a suite launcher portfolio. */
    umi_product_adoption_registry_init(&adoption_registry);
    UMI_TEST_REQUIRE(umi_product_adoption_registry_register(
        &adoption_registry, adoption) == UMI_STATUS_OK);
    UMI_TEST_REQUIRE(umi_product_workspace_guide_portfolio_build(
        &adoption_registry, guide_portfolio) == UMI_STATUS_OK);
    UMI_TEST_REQUIRE(guide_portfolio->application_count == 1U);
    portfolio_summary = umi_product_workspace_guide_portfolio_find(
        guide_portfolio, adoption->application_id);
    UMI_TEST_REQUIRE(portfolio_summary != NULL);
    UMI_TEST_REQUIRE(portfolio_summary->layout_choice_count ==
           workspace_guide->choice_count);
    UMI_TEST_REQUIRE(strcmp(portfolio_summary->recommended_layout_id,
                  workspace_guide->recommended_layout_id) == 0);
    free(guide_portfolio);
    free(workspace_guide);
    free(session_snapshot);
    free(session);
    free(snapshot);
    return 0;
}
