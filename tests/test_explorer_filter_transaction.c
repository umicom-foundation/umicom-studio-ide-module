/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_explorer_filter_transaction.c
 * PURPOSE: Verify Studio publishes centre, service, workspace and projection query state together.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#define umi_studio_test_explorer_centre_create CentreFixture_umi_studio_test_explorer_centre_create
#define umi_studio_test_explorer_centre_create_bound CentreFixture_umi_studio_test_explorer_centre_create_bound
#define umi_studio_test_explorer_centre_destroy CentreFixture_umi_studio_test_explorer_centre_destroy
#define umi_studio_test_explorer_centre_snapshot CentreFixture_umi_studio_test_explorer_centre_snapshot
#define umi_studio_test_explorer_centre_service CentreFixture_umi_studio_test_explorer_centre_service
#define umi_studio_test_explorer_centre_workspace CentreFixture_umi_studio_test_explorer_centre_workspace
#define umi_studio_test_explorer_centre_experience CentreFixture_umi_studio_test_explorer_centre_experience
#define umi_studio_test_explorer_centre_set_workspace CentreFixture_umi_studio_test_explorer_centre_set_workspace
#define umi_studio_test_explorer_centre_set_filter CentreFixture_umi_studio_test_explorer_centre_set_filter
#define umi_studio_test_explorer_centre_hierarchy CentreFixture_umi_studio_test_explorer_centre_hierarchy
#define umi_studio_test_explorer_centre_plan_all CentreFixture_umi_studio_test_explorer_centre_plan_all
#define umi_studio_test_explorer_centre_plan_failed CentreFixture_umi_studio_test_explorer_centre_plan_failed
#define umi_studio_test_explorer_centre_begin CentreFixture_umi_studio_test_explorer_centre_begin
#define umi_studio_test_explorer_centre_stop CentreFixture_umi_studio_test_explorer_centre_stop
#define umi_studio_test_explorer_centre_finish CentreFixture_umi_studio_test_explorer_centre_finish
#define umi_studio_test_explorer_command_contribution_count CentreFixture_umi_studio_test_explorer_command_contribution_count
#define umi_studio_test_explorer_command_contribution_at CentreFixture_umi_studio_test_explorer_command_contribution_at
#define umi_studio_test_explorer_command_contribution_find CentreFixture_umi_studio_test_explorer_command_contribution_find
#define umi_studio_test_explorer_view_contribution_count CentreFixture_umi_studio_test_explorer_view_contribution_count
#define umi_studio_test_explorer_view_contribution_at CentreFixture_umi_studio_test_explorer_view_contribution_at
#define umi_studio_test_explorer_view_contribution_find CentreFixture_umi_studio_test_explorer_view_contribution_find
#include "umicom/studio/test_explorer_centre.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static UmiStatus refusal = UMI_STATUS_OK;
static UmiStatus CentreFixtureApply(UmiStudioTestService *service, UmiTestExplorerSession *session,
    const UmiTestPlatformFilter *filter, UmiTestPlatformFilterSummary *summary)
{
    if (refusal != UMI_STATUS_OK) return refusal;
    return UmiStudioTestServiceApplyExplorerFilter(service, session, filter, summary);
}
/* Only the composition boundary is intercepted. Success exercises the real
 * Studio adapter and Framework; failure proves there was no earlier mutation. */
#define UmiStudioTestServiceApplyExplorerFilter CentreFixtureApply
#include "../src/app/test_explorer_centre.c"
#undef UmiStudioTestServiceApplyExplorerFilter
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); failed = 1; goto done; } } while (0)
int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    const char *names[] = {"coherent", "clear", "long-text", "long-label", "invalid-outcome",
        "binding", "unbound", "allocation-refusal", "revision-overflow", "null-strings", "catalogue-snapshot"};
    int mode = -1, failed = 0;
    for (size_t i = 0U; i < sizeof names / sizeof names[0]; ++i)
        if (strcmp(argv[1], names[i]) == 0) mode = (int)i;
    if (mode < 0) return 2;
    UmiStudioTestService *service = NULL, *other = NULL;
    UmiStudioTestExplorerCentre *centre = NULL;
    UmiStudioTestExplorerState state_before, state_after;
    UmiTestExplorerSessionSnapshot *before = calloc(1U, sizeof *before);
    UmiTestExplorerSessionSnapshot *after = calloc(1U, sizeof *after);
    CHECK(before != NULL && after != NULL);
    if (mode == 6) CHECK(umi_studio_test_explorer_centre_create(&centre) == UMI_STATUS_OK);
    else {
        CHECK(umi_studio_test_service_create(&service) == UMI_STATUS_OK);
        CHECK(umi_studio_test_explorer_centre_create_bound(service, &centre) == UMI_STATUS_OK);
    }
    UmiTestPlatformService *platform = umi_studio_test_explorer_centre_service(centre);
    UmiTestExplorerSession *session = umi_studio_test_explorer_centre_experience(centre);
    UmiTestPlatformItemSnapshot item = {0}; strcpy(item.kind, "test"); item.enabled = 1;
    strcpy(item.id, "alpha"); strcpy(item.name, "Alpha");
    CHECK(umi_test_platform_item_registry_upsert(umi_test_platform_service_item(platform), &item) == UMI_STATUS_OK);
    strcpy(item.id, "beta"); strcpy(item.name, "Beta");
    CHECK(umi_test_platform_item_registry_upsert(umi_test_platform_service_item(platform), &item) == UMI_STATUS_OK);
    if (mode == 10) {
        for (size_t i = 0U; i < 1025U; ++i) {
            snprintf(item.id, sizeof item.id, "extra.%04zu", i);
            CHECK(umi_test_platform_item_registry_upsert(umi_test_platform_service_item(platform), &item) == UMI_STATUS_OK);
        }
        UmiStudioTestExplorerCentreSnapshot *snapshot = calloc(1U, sizeof *snapshot);
        CHECK(snapshot != NULL);
        UmiStatus captured = umi_studio_test_explorer_centre_snapshot(centre, snapshot);
        size_t count = snapshot->hierarchy_count;
        free(snapshot);
        CHECK(captured == UMI_STATUS_OK && count == 1027U);
        goto done;
    }
    CHECK(umi_studio_test_explorer_centre_set_filter(centre, "", "", -1, 0) == UMI_STATUS_OK);
    if (mode == 1) CHECK(umi_studio_test_explorer_centre_set_filter(centre, "beta", "", -1, 0) == UMI_STATUS_OK);
    if (mode == 8) centre->revision = UINT64_MAX;
    const uint64_t prior_revision = centre->revision;
    CHECK(umi_test_explorer_session_snapshot(session, before) == UMI_STATUS_OK);
    if (service != NULL) CHECK(umi_studio_test_service_explorer_state(service, &state_before) == UMI_STATUS_OK);
    char too_long[300]; memset(too_long, 'x', sizeof too_long - 1U); too_long[sizeof too_long - 1U] = 0;
    UmiStatus status;
    if (mode == 5) {
        CHECK(umi_studio_test_service_create(&other) == UMI_STATUS_OK);
        UmiTestPlatformFilter filter; umi_test_platform_filter_init(&filter);
        UmiTestPlatformFilterSummary summary = {0}; summary.matched_count = 55U;
        status = UmiStudioTestServiceApplyExplorerFilter(other, session, &filter, &summary);
        CHECK(summary.matched_count == 55U);
    } else {
        refusal = mode == 7 ? UMI_STATUS_OUT_OF_MEMORY : UMI_STATUS_OK;
        status = umi_studio_test_explorer_centre_set_filter(centre,
            mode == 2 ? too_long : mode == 9 ? NULL : mode == 1 ? "" : "beta",
            mode == 3 ? too_long : mode == 9 ? NULL : "", mode == 4 ? 99 : -1, 0);
        refusal = UMI_STATUS_OK;
    }
    CHECK(umi_test_explorer_session_snapshot(session, after) == UMI_STATUS_OK);
    if (service != NULL) CHECK(umi_studio_test_service_explorer_state(service, &state_after) == UMI_STATUS_OK);
    if (mode == 2 || mode == 3 || mode == 4 || mode == 5 || mode == 7 || mode == 8) {
        UmiStatus expected = mode == 7 ? UMI_STATUS_OUT_OF_MEMORY :
            (mode == 4 || mode == 5) ? UMI_STATUS_INVALID_ARGUMENT : UMI_STATUS_CAPACITY_EXCEEDED;
        CHECK(status == expected && centre->revision == prior_revision);
        CHECK(memcmp(before, after, sizeof *before) == 0);
        CHECK(memcmp(&state_before, &state_after, sizeof state_before) == 0);
    } else {
        size_t expected = mode == 1 || mode == 9 ? 2U : 1U;
        CHECK(status == UMI_STATUS_OK && centre->revision == prior_revision + 1U);
        CHECK(after->workspace.visible_item_count == expected && after->explorer.visible_row_count == expected);
        CHECK(after->revision == before->revision + 1U);
        CHECK(after->workspace.revision == before->workspace.revision + 1U);
        CHECK(after->explorer.revision == before->explorer.revision + 1U);
        CHECK(strcmp(after->workspace.filter.text, after->explorer.filter.text) == 0);
        if (service != NULL) {
            CHECK(state_after.revision == state_before.revision + 1U && state_after.selected_count == expected);
            CHECK(strcmp(state_after.search_text, after->explorer.filter.text) == 0);
        }
    }
done:
    refusal = UMI_STATUS_OK;
    umi_studio_test_explorer_centre_destroy(centre);
    umi_studio_test_service_destroy(service);
    umi_studio_test_service_destroy(other);
    free(before); free(after);
    return failed;
}
