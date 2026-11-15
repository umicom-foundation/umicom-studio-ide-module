/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_ai_context_preview.c
 *
 * PURPOSE:
 *   Verify Studio delegates complete context previews and stale ownership checks to Framework without model execution.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "umicom/studio/ai_platform.h"
#define REQUIRE(condition) do { if (!(condition)) { \
    fprintf(stderr, "Studio context preview check failed at line %d\n", __LINE__); \
    result = 1; goto cleanup; } } while (0)
#define CASE(name) (strcmp(argv[1], name) == 0)
int main(int argc, char **argv)
{
    static const char *const modes[] = {"preview",
        "context-stale",
        "policy-stale",
        "runtime-stale",
        "missing-runtime",
        "null-platform",
        "null-plan",
        "null-observation",
        "check-null-platform",
        "check-null-observation",
        "refresh"};
    UmiStudioAiPlatformConfig config = umi_studio_ai_platform_config_default();
    UmiStudioAiPlatform *platform = NULL;
    UmiAiAuthorEngineService *engine;
    UmiAiContextPlan *plan = malloc(sizeof *plan), *prior_plan = malloc(sizeof *prior_plan);
    UmiAiContextPlanObservation observation, prior_observation;
    UmiAiRuntimeDescriptor descriptor;
    UmiAiAuthorEngineServiceSnapshot before_snapshot, after_snapshot;
    UmiStatus status, expected = UMI_STATUS_OK;
    int result = 0, known = 0;
    REQUIRE(argc == 2 && plan != NULL && prior_plan != NULL);
    for (size_t i = 0U; i < sizeof modes / sizeof modes[0]; ++i)
        if (CASE(modes[i])) known = 1;
    REQUIRE(known);
    config.allow_remote = 0; config.persist_sessions = 0;
    status = umi_studio_ai_platform_create_configured(&config, &platform);
    REQUIRE(status == UMI_STATUS_OK);
    engine = umi_studio_ai_platform_authorengine(platform);
    REQUIRE(engine != NULL);
    status = umi_ai_runtime_catalogue_at(umi_ai_authorengine_service_catalogue(engine), 0U, &descriptor);
    REQUIRE(status == UMI_STATUS_OK);
    memset(plan, 0xA5, sizeof *plan); memcpy(prior_plan, plan, sizeof *plan);
    memset(&observation, 0x5A, sizeof observation); memcpy(&prior_observation, &observation, sizeof observation);
    status = UmiStudioAiPlatformPlanContextObserved(CASE("null-platform") ? NULL : platform,
        CASE("missing-runtime") ? "runtime.missing" : descriptor.runtime_id, 1,
        CASE("null-plan") ? NULL : plan, CASE("null-observation") ? NULL : &observation);
    if (CASE("missing-runtime") || CASE("null-platform") || CASE("null-plan") || CASE("null-observation")) {
        REQUIRE(status == (CASE("missing-runtime") ? UMI_STATUS_NOT_FOUND : UMI_STATUS_INVALID_ARGUMENT));
        REQUIRE(memcmp(prior_plan, plan, sizeof *plan) == 0);
        REQUIRE(memcmp(&prior_observation, &observation, sizeof observation) == 0);
        goto cleanup;
    }
    REQUIRE(status == UMI_STATUS_OK && plan->revision == observation.context_revision);
    REQUIRE(UmiStudioAiPlatformCheckContextObservation(platform, &observation) == UMI_STATUS_OK);
    if (CASE("context-stale")) {
        status = UmiAiContextBrokerClearChecked(umi_ai_authorengine_service_context(engine));
        REQUIRE(status == UMI_STATUS_OK); expected = UMI_STATUS_BUSY;
    }
    if (CASE("policy-stale") || CASE("refresh")) {
        UmiAiPrivacyPolicy privacy = *umi_ai_authorengine_service_privacy_policy(engine);
        UmiAiPolicy provider = umi_studio_ai_platform_runtime(platform)->policy;
        status = umi_ai_authorengine_service_set_policy(engine, &provider, &privacy,
            config.context_tokens, config.reserved_output_tokens);
        REQUIRE(status == UMI_STATUS_OK); expected = UMI_STATUS_BUSY;
    }
    if (CASE("runtime-stale")) {
        strcpy(descriptor.model_id, "model.changed.after.preview");
        status = umi_ai_runtime_catalogue_upsert(umi_ai_authorengine_service_catalogue(engine), &descriptor);
        REQUIRE(status == UMI_STATUS_OK); expected = UMI_STATUS_BUSY;
    }
    if (CASE("check-null-platform") || CASE("check-null-observation")) expected = UMI_STATUS_INVALID_ARGUMENT;
    status = umi_studio_ai_platform_snapshot(platform, &before_snapshot);
    REQUIRE(status == UMI_STATUS_OK);
    memcpy(prior_plan, plan, sizeof *plan); memcpy(&prior_observation, &observation, sizeof observation);
    status = UmiStudioAiPlatformCheckContextObservation(CASE("check-null-platform") ? NULL : platform,
        CASE("check-null-observation") ? NULL : &observation);
    REQUIRE(status == expected);
    status = umi_studio_ai_platform_snapshot(platform, &after_snapshot);
    REQUIRE(status == UMI_STATUS_OK && memcmp(&before_snapshot, &after_snapshot, sizeof before_snapshot) == 0);
    REQUIRE(memcmp(prior_plan, plan, sizeof *plan) == 0);
    REQUIRE(memcmp(&prior_observation, &observation, sizeof observation) == 0);
    REQUIRE(!umi_studio_ai_platform_runtime(platform)->policy.allow_remote);
    REQUIRE(!umi_ai_authorengine_service_privacy_policy(engine)->persist_sessions);
    if (CASE("refresh")) {
        status = UmiStudioAiPlatformPlanContextObserved(platform, observation.runtime_id, 1, plan, &observation);
        REQUIRE(status == UMI_STATUS_OK && observation.service_revision != prior_observation.service_revision);
        REQUIRE(UmiStudioAiPlatformCheckContextObservation(platform, &observation) == UMI_STATUS_OK);
    }
cleanup:
    umi_studio_ai_platform_destroy(platform); free(plan); free(prior_plan);
    return result;
}
#undef CASE
#undef REQUIRE
