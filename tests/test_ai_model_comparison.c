/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_ai_model_comparison.c
 *
 * PURPOSE:
 *   Verify Studio delegates model comparison to the shared Framework runtime.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* BEGINNER NOTE:
 * Studio supplies the prompt and target choice. Framework owns provider lookup,
 * approval policy, response collection and result selection.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "umicom/studio/ai_platform.h"

static UmiStatus second_provider_generate(
    void *instance,
    const UmiAiRequest *request,
    UmiAiResponse *response)
{
    (void)instance;
    if (request == NULL || response == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    /* A second deterministic adapter proves Studio can compare registrations. */
    umi_ai_response_init(response);
    (void)snprintf(
        response->provider_id, sizeof(response->provider_id),
        "%s", "test.second");
    (void)snprintf(
        response->model_id, sizeof(response->model_id),
        "%s", "second-model");
    return umi_ai_response_set_text(response, "A second provider answer");
}

int main(void)
{
    UmiStudioAiPlatform *platform = NULL;
    UmiAiProvider provider;
    UmiAiModelTarget targets[2];
    const UmiAiModelEnsembleReport *report;
    char first_request_id[UMI_AI_ID_CAPACITY];

    assert(umi_studio_ai_platform_create(&platform) == UMI_STATUS_OK);
    (void)memset(&provider, 0, sizeof(provider));
    provider.structure_size = (uint32_t)sizeof(provider);
    provider.abi_version = 1U;
    provider.provider_id = "test.second";
    provider.kind = UMI_AI_PROVIDER_TEST;
    provider.generate = second_provider_generate;
    assert(umi_studio_ai_platform_register_provider(
               platform, &provider) == UMI_STATUS_OK);
    assert(umi_ai_model_target_initialize(
               &targets[0], "studio.local", "studio-reference",
               "Studio Local", 0) == UMI_STATUS_OK);
    assert(umi_ai_model_target_initialize(
               &targets[1], "test.second", "second-model",
               "Second Test Provider", 0) == UMI_STATUS_OK);
    assert(umi_studio_ai_platform_compare_models(
               platform, "Explain a safe refactoring", targets, 2U) ==
           UMI_STATUS_OK);
    report = umi_studio_ai_platform_model_comparison(platform);
    assert(report != NULL);
    assert(report->result_count == 2U);
    assert(report->success_count == 2U);
    assert(strstr(report->results[0].response.text, "safe refactoring") != NULL);
    assert(strstr(report->results[1].response.text, "second provider") != NULL);
    (void)snprintf(
        first_request_id, sizeof(first_request_id),
        "%s", report->request_id);

    /* Separate comparisons receive separate IDs for logs and audit evidence. */
    assert(umi_studio_ai_platform_compare_models(
               platform, "Explain a second change", targets, 2U) ==
           UMI_STATUS_OK);
    report = umi_studio_ai_platform_model_comparison(platform);
    assert(strcmp(first_request_id, report->request_id) != 0);
    umi_studio_ai_platform_destroy(platform);
    return 0;
}
