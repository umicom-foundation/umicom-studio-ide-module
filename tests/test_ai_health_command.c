/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_ai_health_command.c
 *
 * PURPOSE:
 *   Verify Studio AI health feedback through the real Framework command registry and isolated services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Keep setup and command assertions active in optimized test builds. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "studio_test_state.h"
#include "umicom/studio/commands.h"
#include "umicom/studio/ai_platform.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef struct HealthCommandFixture {
    UmiStudioServices *services;
    UmiCommandRegistry *commands;
    const char *mode;
    unsigned calls;
} HealthCommandFixture;

static UmiStatus never_generate(void *context, const UmiAiRequest *request, UmiAiResponse *response)
{
    (void)context; (void)request; (void)response;
    assert(!"health feedback must not generate model text");
    return UMI_STATUS_NOT_IMPLEMENTED;
}

static UmiStatus failing_health(void *context, UmiAiHealth *health)
{
    HealthCommandFixture *fixture = context;
    ++fixture->calls;
    if (strcmp(fixture->mode, "stale-runtime") == 0) {
        UmiAiAuthorEngineService *service = umi_studio_ai_platform_authorengine(
            umi_studio_services_ai_platform(fixture->services));
        UmiAiRuntimeDescriptor value;
        assert(umi_ai_runtime_catalogue_find(umi_ai_authorengine_service_catalogue(service),
            "fixture.runtime", &value) == UMI_STATUS_OK);
        strcpy(value.model_id, "replacement");
        assert(umi_ai_authorengine_service_register_runtime(service, &value) == UMI_STATUS_OK);
    } else {
        assert(strcmp(fixture->mode, "invalid-health") == 0);
        strcpy(health->provider_id, "unrelated.provider");
    }
    health->available = 1;
    return UMI_STATUS_OK;
}

static UmiStatus command_during_probe(void *context, const UmiAiRuntimeDescriptor *descriptor,
                                      UmiAiHealth *health)
{
    HealthCommandFixture *fixture = context;
    if (fixture->calls++ == 0U) {
        char message[256];
        UmiAiRuntimeDescriptor before, after;
        UmiAiRuntimeCatalogue *catalogue = umi_ai_authorengine_service_catalogue(
            umi_studio_ai_platform_authorengine(umi_studio_services_ai_platform(fixture->services)));
        assert(umi_ai_runtime_catalogue_find(catalogue, descriptor->runtime_id, &before) == UMI_STATUS_OK);
        assert(umi_command_registry_execute(fixture->commands, UMI_STUDIO_COMMAND_AI_REFRESH_HEALTH,
            NULL, message, sizeof message) == UMI_STATUS_BUSY);
        assert(strstr(message, "not completed") != NULL);
        assert(strstr(message, umi_status_text(UMI_STATUS_BUSY)) != NULL);
        assert(strstr(message, "refreshed:") == NULL);
        assert(umi_ai_runtime_catalogue_find(catalogue, descriptor->runtime_id, &after) == UMI_STATUS_OK);
        assert(memcmp(&before, &after, sizeof before) == 0);
    }
    health->available = 0;
    return UMI_STATUS_OK;
}

int main(int argc, char **argv)
{
    StudioTestState state = {0};
    UmiStudioServices *services = NULL;
    UmiCommandRegistry *commands = NULL;
    UmiStudioServicesOptions options = {0};
    HealthCommandFixture fixture = {0};
    UmiAiRuntimeDescriptor before, after, reference_before, reference_after;
    UmiStatus expected = UMI_STATUS_OK, status;
    char message[256] = "previous feedback";
    assert(argc == 2);
    fixture.mode = argv[1];
    assert(strcmp(argv[1], "success") == 0 || strcmp(argv[1], "invalid-health") == 0 ||
        strcmp(argv[1], "stale-runtime") == 0 || strcmp(argv[1], "clock-missing") == 0 ||
        strcmp(argv[1], "busy") == 0 || strcmp(argv[1], "short-message") == 0 ||
        strcmp(argv[1], "no-message") == 0);
    assert(StudioTestStateBegin(&state));
    assert(umi_studio_services_create_with_options(NULL, NULL, &options, &services) == UMI_STATUS_OK);
    assert(umi_command_registry_create(&commands) == UMI_STATUS_OK);
    assert(umi_studio_commands_register(commands, services) == UMI_STATUS_OK);
    fixture.services = services; fixture.commands = commands;
    UmiStudioAiPlatform *platform = umi_studio_services_ai_platform(services);
    UmiAiAuthorEngineService *service = umi_studio_ai_platform_authorengine(platform);
    UmiAiRuntimeCatalogue *catalogue = umi_ai_authorengine_service_catalogue(service);
    assert(umi_ai_runtime_catalogue_find(catalogue, "studio.local.reference", &reference_before) == UMI_STATUS_OK);
    int fault = strcmp(argv[1], "invalid-health") == 0 || strcmp(argv[1], "stale-runtime") == 0;
    if (fault) {
        UmiAiProvider provider = {0};
        UmiAiRuntimeDescriptor value = reference_before;
        provider.structure_size = sizeof provider; provider.abi_version = 1U;
        provider.provider_id = "fixture.health"; provider.kind = UMI_AI_PROVIDER_TEST;
        provider.instance = &fixture; provider.generate = never_generate; provider.health = failing_health;
        assert(umi_studio_ai_platform_register_provider(platform, &provider) == UMI_STATUS_OK);
        strcpy(value.runtime_id, "fixture.runtime");
        strcpy(value.provider_id, "fixture.health");
        strcpy(value.health.provider_id, "fixture.health");
        value.health.available = 0; value.health.checked_at_ns = 0U;
        assert(umi_ai_authorengine_service_register_runtime(service, &value) == UMI_STATUS_OK);
        assert(umi_ai_runtime_catalogue_find(catalogue, "fixture.runtime", &before) == UMI_STATUS_OK);
        expected = strcmp(argv[1], "invalid-health") == 0 ? UMI_STATUS_INVALID_ARGUMENT : UMI_STATUS_BUSY;
    }
    UmiClock *clock = umi_studio_services_clock(services);
    assert(clock != NULL);
    UmiClock saved_clock = *clock;
    if (strcmp(argv[1], "clock-missing") == 0) {
        clock->wall_nanoseconds = NULL;
        expected = UMI_STATUS_INVALID_STATE;
    }
    if (strcmp(argv[1], "busy") == 0) {
        size_t count = SIZE_MAX;
        assert(UmiAiAuthorEngineRefreshRuntimeHealth(service, 123U,
            command_during_probe, &fixture, &count) == UMI_STATUS_OK);
        assert(count == 0U && fixture.calls > 0U);
        /* After the outer probe returns, the ordinary command can run again. */
        status = umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_AI_REFRESH_HEALTH,
            NULL, message, sizeof message);
    } else if (strcmp(argv[1], "short-message") == 0) {
        char guarded[3] = {'L', 'X', 'R'};
        status = umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_AI_REFRESH_HEALTH,
            NULL, &guarded[1], 1U);
        assert(guarded[0] == 'L' && guarded[1] == '\0' && guarded[2] == 'R');
    } else {
        int no_message = strcmp(argv[1], "no-message") == 0;
        status = umi_command_registry_execute(commands, UMI_STUDIO_COMMAND_AI_REFRESH_HEALTH,
            NULL, no_message ? NULL : message, no_message ? 0U : sizeof message);
    }
    *clock = saved_clock;
    assert(status == expected);
    if (expected != UMI_STATUS_OK) {
        assert(strstr(message, "not completed") != NULL);
        assert(strstr(message, umi_status_text(expected)) != NULL);
        assert(strstr(message, "refreshed:") == NULL);
        assert(strstr(message, "Earlier observations may have changed") != NULL);
    } else if (strcmp(argv[1], "short-message") != 0 && strcmp(argv[1], "no-message") != 0) {
        assert(strstr(message, "AI health refreshed:") != NULL);
        assert(strstr(message, "Model identity is unverified") != NULL);
    }
    assert(umi_ai_runtime_catalogue_find(catalogue, "studio.local.reference", &reference_after) == UMI_STATUS_OK);
    if (strcmp(argv[1], "clock-missing") == 0)
        assert(memcmp(&reference_before, &reference_after, sizeof reference_before) == 0);
    else
        assert(reference_after.revision > reference_before.revision);
    if (fault) {
        assert(fixture.calls == 1U);
        assert(umi_ai_runtime_catalogue_find(catalogue, "fixture.runtime", &after) == UMI_STATUS_OK);
        if (expected == UMI_STATUS_INVALID_ARGUMENT)
            assert(memcmp(&before, &after, sizeof before) == 0);
        else
            assert(strcmp(after.model_id, "replacement") == 0 && !after.health.available);
    }
    umi_command_registry_destroy(commands);
    assert(UmiStudioServicesRelease(&services) == UMI_STATUS_OK);
    assert(services == NULL);
    assert(StudioTestStateEnd(&state));
    return 0;
}
