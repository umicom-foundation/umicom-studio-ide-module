/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_ai_parent_lifetime.c
 *
 * PURPOSE:
 *   Verify retained Studio parents during patch callbacks and provider destruction reentry.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Include the service composition solely to construct an isolated partial
 * owner. No background worker, session file, provider process or GUI starts. */
#include "../src/app/services.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(value) do { if (!(value)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #value); goto cleanup; \
} } while (0)

typedef struct ParentFixture {
    UmiStudioServices *services;
    UmiStudioAiPlatform *platform;
    UmiAiCodingAssistantService *coding;
    UmiAiCodingPatch *patch;
    const char *hook, *phase;
    int entered, failed, writes, fail_first_write, destroy_calls;
    char text[64];
} ParentFixture;

/* Exercise direct AuthorEngine cleanup as well as Studio parent cleanup from health callbacks. The former implementation is retained for owner review. */
#if 0
static UmiStatus attempt_release(ParentFixture *f)
{
    if (strcmp(f->hook, "framework-idle") == 0)
        return UmiAiCodingAssistantCheckIdle(f->coding);
    if (strcmp(f->hook, "platform-idle") == 0)
        return UmiStudioAiPlatformCheckIdle(f->platform);
    if (strcmp(f->hook, "platform-destroy") == 0)
        return UmiStudioAiPlatformDestroyChecked(f->platform);
    if (strcmp(f->hook, "platform-legacy") == 0) {
        umi_studio_ai_platform_destroy(f->platform);
        return UmiStudioAiPlatformCheckIdle(f->platform);
    }
    if (strcmp(f->hook, "services-release") == 0) {
        UmiStudioServices *alias = f->services;
        UmiStatus status = UmiStudioServicesRelease(&alias);
        if (alias != f->services) f->failed = 1;
        return status;
    }
    if (strcmp(f->hook, "services-legacy") == 0) {
        umi_studio_services_destroy(f->services);
        return UmiStudioAiPlatformCheckIdle(f->services->ai_platform);
    }
    if (strcmp(f->hook, "services-stop") == 0)
        return UmiStudioServicesStopBackground(f->services);
    return UMI_STATUS_INVALID_ARGUMENT;
}

#endif
static UmiStatus attempt_release(ParentFixture *f)
{
    if (strcmp(f->hook, "framework-idle") == 0)
        return UmiAiCodingAssistantCheckIdle(f->coding);
    if (strcmp(f->hook, "platform-idle") == 0)
        return UmiStudioAiPlatformCheckIdle(f->platform);
    if (strcmp(f->hook, "platform-destroy") == 0)
        return UmiStudioAiPlatformDestroyChecked(f->platform);
    if (strcmp(f->hook, "platform-legacy") == 0) {
        umi_studio_ai_platform_destroy(f->platform);
        return UmiStudioAiPlatformCheckIdle(f->platform);
    }
    if (strcmp(f->hook, "services-release") == 0) {
        UmiStudioServices *alias = f->services;
        UmiStatus status = UmiStudioServicesRelease(&alias);
        if (alias != f->services) f->failed = 1;
        return status;
    }
    if (strcmp(f->hook, "services-legacy") == 0) {
        umi_studio_services_destroy(f->services);
        return UmiStudioAiPlatformCheckIdle(f->services->ai_platform);
    }
    if (strcmp(f->hook, "services-stop") == 0)
        return UmiStudioServicesStopBackground(f->services);
    if (strcmp(f->hook, "authorengine-idle") == 0)
        return UmiAiAuthorEngineServiceCheckIdle(umi_studio_ai_platform_authorengine(f->platform));
    if (strcmp(f->hook, "authorengine-destroy") == 0)
        return UmiAiAuthorEngineServiceDestroyChecked(umi_studio_ai_platform_authorengine(f->platform));
    if (strcmp(f->hook, "authorengine-legacy") == 0) {
        umi_ai_authorengine_service_destroy(umi_studio_ai_platform_authorengine(f->platform));
        return UmiAiAuthorEngineServiceCheckIdle(umi_studio_ai_platform_authorengine(f->platform));
    }
    return UMI_STATUS_INVALID_ARGUMENT;
}

static void patch_hook(ParentFixture *f, const char *phase)
{
    if (f->hook == NULL || f->entered || strcmp(f->phase, phase) != 0) return;
    f->entered = 1;
    UmiStudioServices before;
    UmiAiCodingAssistantSnapshot before_coding, after_coding;
    memcpy(&before, f->services, sizeof before);
    if (umi_ai_coding_assistant_snapshot(f->coding, &before_coding) != UMI_STATUS_OK ||
        attempt_release(f) != UMI_STATUS_BUSY ||
        memcmp(&before, f->services, sizeof before) != 0 ||
        umi_studio_ai_platform_coding_assistant(f->platform) != f->coding ||
        umi_ai_coding_assistant_snapshot(f->coding, &after_coding) != UMI_STATUS_OK ||
        memcmp(&before_coding, &after_coding, sizeof before_coding) != 0)
        f->failed = 1;
}

static UmiStatus memory_read(void *user, const char *path, char *out,
    size_t capacity, size_t *length)
{
    ParentFixture *f = user;
    (void)path; patch_hook(f, "read"); *length = strlen(f->text);
    if (*length >= capacity) return UMI_STATUS_CAPACITY_EXCEEDED;
    memcpy(out, f->text, *length); return UMI_STATUS_OK;
}

static UmiStatus memory_write(void *user, const char *path, const char *text, size_t length)
{
    ParentFixture *f = user;
    (void)path; ++f->writes;
    patch_hook(f, f->writes == 1 ? "write" : "compensation");
    if (f->fail_first_write && f->writes == 1) return UMI_STATUS_IO_ERROR;
    if (length >= sizeof f->text) return UMI_STATUS_CAPACITY_EXCEEDED;
    memcpy(f->text, text, length); f->text[length] = '\0'; return UMI_STATUS_OK;
}

static UmiStatus memory_remove(void *user, const char *path)
{
    (void)user; (void)path; return UMI_STATUS_NOT_FOUND;
}

static UmiStatus fixture_init(ParentFixture *f)
{
    UmiStudioAiPlatformConfig config = umi_studio_ai_platform_config_default();
    config.allow_remote = 0; config.persist_sessions = 0;
    config.knowledge_source_capacity = 4U; config.knowledge_vector_capacity = 16U;
    f->services = calloc(1U, sizeof *f->services);
    f->patch = malloc(sizeof *f->patch);
    if (f->services == NULL || f->patch == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    UmiStatus status = umi_studio_ai_platform_create_configured(&config, &f->platform);
    if (status != UMI_STATUS_OK) return status;
    f->services->ai_platform = f->platform;
    f->coding = umi_studio_ai_platform_coding_assistant(f->platform);
    strcpy(f->text, "before");
    status = umi_ai_coding_patch_init(f->patch, "patch.parent", "request.parent",
        "Parent lifetime", "Retain owners through callbacks");
    if (status == UMI_STATUS_OK) status = umi_ai_coding_patch_add_file(f->patch,
        "src/example.c", UMI_AI_CODING_PATCH_MODIFY, "before", "after");
    if (status == UMI_STATUS_OK) status = umi_ai_coding_assistant_record_patch(f->coding, f->patch);
    if (status == UMI_STATUS_OK) status = umi_ai_coding_assistant_approve_patch(
        f->coding, "patch.parent", "reviewer");
    return status;
}

static void fixture_clear(ParentFixture *f)
{
    f->hook = NULL;
    if (f->services != NULL) (void)UmiStudioServicesRelease(&f->services);
    free(f->patch);
}

static int patch_case(const char *hook, const char *phase, int reverting)
{
    ParentFixture f = {0};
    int result = 1;
    REQUIRE(fixture_init(&f) == UMI_STATUS_OK);
    UmiAiCodingFileAdapter adapter = {sizeof adapter, UMI_AI_CODING_ABI_VERSION,
        memory_read, memory_write, memory_remove, &f};
    REQUIRE(UmiAiCodingAssistantCheckIdle(f.coding) == UMI_STATUS_OK);
    REQUIRE(UmiStudioAiPlatformCheckIdle(f.platform) == UMI_STATUS_OK);
    if (reverting) REQUIRE(umi_ai_coding_assistant_apply_patch(f.coding, "patch.parent", &adapter) == UMI_STATUS_OK);
    f.writes = 0; f.hook = hook; f.phase = phase;
    f.fail_first_write = strcmp(phase, "compensation") == 0;
    UmiStatus status = reverting ?
        umi_ai_coding_assistant_revert_patch(f.coding, "patch.parent", &adapter) :
        umi_ai_coding_assistant_apply_patch(f.coding, "patch.parent", &adapter);
    REQUIRE(status == (f.fail_first_write ? UMI_STATUS_IO_ERROR : UMI_STATUS_OK));
    REQUIRE(f.entered && !f.failed);
    REQUIRE(strcmp(f.text, f.fail_first_write ? (reverting ? "after" : "before") : (reverting ? "before" : "after")) == 0);
    REQUIRE(UmiAiCodingAssistantCheckIdle(f.coding) == UMI_STATUS_OK);
    REQUIRE(UmiStudioAiPlatformCheckIdle(f.platform) == UMI_STATUS_OK);
    f.hook = NULL;
    REQUIRE(UmiStudioServicesRelease(&f.services) == UMI_STATUS_OK && f.services == NULL);
    result = 0;
cleanup:
    fixture_clear(&f); return result;
}

static UmiStatus unused_generate(void *instance, const UmiAiRequest *request, UmiAiResponse *response)
{
    (void)instance; (void)request; (void)response; return UMI_STATUS_UNAVAILABLE;
}

static void provider_destroy(void *instance)
{
    ParentFixture *f = instance;
    ++f->destroy_calls;
    if (f->hook != NULL && attempt_release(f) != UMI_STATUS_BUSY) f->failed = 1;
}

static int provider_case(const char *hook)
{
    ParentFixture f = {0};
    int result = 1;
    REQUIRE(fixture_init(&f) == UMI_STATUS_OK);
    UmiAiProvider provider = {sizeof provider, 1U, "test.parent.release", UMI_AI_PROVIDER_TEST,
        &f, unused_generate, NULL, provider_destroy};
    REQUIRE(umi_ai_provider_registry_add(&umi_studio_ai_platform_runtime(f.platform)->providers,
        &provider) == UMI_STATUS_OK);
    f.hook = hook;
    REQUIRE(UmiStudioServicesRelease(&f.services) == UMI_STATUS_OK && f.services == NULL);
    REQUIRE(f.destroy_calls == 1 && !f.failed);
    result = 0;
cleanup:
    fixture_clear(&f); return result;
}

static int admission_case(const char *mode)
{
    ParentFixture f = {0};
    UmiAiProviderRegistry *registry = NULL;
    size_t prior_count = 0U;
    int result = 1;
    if (strcmp(mode, "null") == 0) {
        REQUIRE(UmiAiCodingAssistantCheckIdle(NULL) == UMI_STATUS_OK);
        REQUIRE(UmiStudioAiPlatformCheckIdle(NULL) == UMI_STATUS_OK);
        REQUIRE(UmiStudioAiPlatformDestroyChecked(NULL) == UMI_STATUS_OK);
        REQUIRE(UmiStudioServicesRelease(&f.services) == UMI_STATUS_OK);
        REQUIRE(UmiStudioServicesRelease(NULL) == UMI_STATUS_INVALID_ARGUMENT);
        result = 0; goto cleanup;
    }
    REQUIRE(fixture_init(&f) == UMI_STATUS_OK);
    registry = &umi_studio_ai_platform_runtime(f.platform)->providers;
    prior_count = registry->count;
    if (strcmp(mode, "registry-count") == 0) {
        UmiStudioServices before;
        memcpy(&before, f.services, sizeof before);
        registry->count = UMI_AI_MAX_PROVIDERS + 1U;
        REQUIRE(UmiStudioAiPlatformCheckIdle(f.platform) == UMI_STATUS_INVALID_STATE);
        REQUIRE(UmiStudioAiPlatformDestroyChecked(f.platform) == UMI_STATUS_INVALID_STATE);
        REQUIRE(UmiStudioServicesStopBackground(f.services) == UMI_STATUS_INVALID_STATE);
        REQUIRE(UmiStudioServicesRelease(&f.services) == UMI_STATUS_INVALID_STATE);
        REQUIRE(memcmp(&before, f.services, sizeof before) == 0);
        REQUIRE(umi_studio_ai_platform_coding_assistant(f.platform) == f.coding);
        registry->count = prior_count;
    } else if (strcmp(mode, "platform-clean") == 0) {
        REQUIRE(UmiStudioAiPlatformDestroyChecked(f.platform) == UMI_STATUS_OK);
        f.services->ai_platform = NULL; f.platform = NULL; registry = NULL;
    } else REQUIRE(strcmp(mode, "partial-clean") == 0);
    REQUIRE(UmiStudioServicesRelease(&f.services) == UMI_STATUS_OK && f.services == NULL);
    registry = NULL;
    result = 0;
cleanup:
    if (registry != NULL) registry->count = prior_count;
    fixture_clear(&f); return result;
}

/* Extend existing parent lifetime qualification to provider generation while retaining patch and destructor cases. The former implementation is retained for owner review. */
#if 0
int main(int argc, char **argv)
{
    if (argc == 5 && strcmp(argv[1], "patch") == 0)
        return patch_case(argv[2], argv[3], strcmp(argv[4], "revert") == 0);
    if (argc == 3 && strcmp(argv[1], "provider") == 0) return provider_case(argv[2]);
    if (argc == 3 && strcmp(argv[1], "admission") == 0) return admission_case(argv[2]);
    return 2;
}

#endif

/* The provider calls back through the real Studio composition. Framework owns
 * the invocation frame; Studio only asks that owner before releasing siblings. */
static UmiStatus provider_generate_reentry(void *instance,
    const UmiAiRequest *request, UmiAiResponse *response)
{
    ParentFixture *f = instance;
    (void)request;
    if (f->entered) return UMI_STATUS_INVALID_STATE;
    f->entered = 1;
    UmiStudioServices before;
    memcpy(&before, f->services, sizeof before);
    if (attempt_release(f) != UMI_STATUS_BUSY || f->destroy_calls != 0 ||
        memcmp(&before, f->services, sizeof before) != 0 ||
        umi_studio_ai_platform_coding_assistant(f->platform) != f->coding ||
        UmiAiRuntimeCheckIdle(umi_studio_ai_platform_runtime(f->platform)) != UMI_STATUS_BUSY)
        f->failed = 1;
    umi_ai_response_init(response);
    strcpy(response->text, "generation completed");
    response->finish_reason = UMI_AI_FINISH_STOP;
    return strcmp(f->phase, "provider-error") == 0 ? UMI_STATUS_IO_ERROR : UMI_STATUS_OK;
}

static int generation_case(const char *hook, const char *entry, const char *outcome)
{
    ParentFixture f = {0};
    UmiAiRequest *request = NULL;
    int result = 1;
    REQUIRE(fixture_init(&f) == UMI_STATUS_OK);
    request = malloc(sizeof *request);
    REQUIRE(request != NULL);
    umi_ai_request_init(request);
    UmiAiProvider provider = {sizeof provider, 1U, "test.parent.generate", UMI_AI_PROVIDER_TEST,
        &f, provider_generate_reentry, NULL, provider_destroy};
    UmiAiRuntime *runtime = umi_studio_ai_platform_runtime(f.platform);
    REQUIRE(umi_ai_provider_registry_add(&runtime->providers, &provider) == UMI_STATUS_OK);
    UmiAiResponse response;
    umi_ai_response_init(&response);
    strcpy(response.text, "previous");
    f.hook = hook; f.phase = outcome;
    UmiStatus expected = strcmp(outcome, "provider-error") == 0 ? UMI_STATUS_IO_ERROR : UMI_STATUS_OK;
    UmiStatus status;
    if (strcmp(entry, "checked") == 0)
        status = UmiAiRuntimeGenerateChecked(runtime, provider.provider_id, 1, request, &response, NULL);
    else {
        REQUIRE(strcmp(entry, "legacy") == 0);
        status = umi_ai_runtime_generate(runtime, provider.provider_id, 1, request, &response);
    }
    REQUIRE(status == expected && f.entered && !f.failed && f.destroy_calls == 0);
    REQUIRE(UmiStudioAiPlatformCheckIdle(f.platform) == UMI_STATUS_OK);
    if (expected != UMI_STATUS_OK && strcmp(entry, "checked") == 0)
        REQUIRE(strcmp(response.text, "previous") == 0);
    f.hook = NULL;
    REQUIRE(UmiStudioServicesRelease(&f.services) == UMI_STATUS_OK && f.services == NULL);
    REQUIRE(f.destroy_calls == 1);
    result = 0;
cleanup:
    free(request); fixture_clear(&f); return result;
}

/* Add real parent health-callback fixtures while retaining every existing patch, destruction and generation case. The former implementation is retained for owner review. */
#if 0
int main(int argc, char **argv)
{
    if (argc == 5 && strcmp(argv[1], "patch") == 0)
        return patch_case(argv[2], argv[3], strcmp(argv[4], "revert") == 0);
    if (argc == 3 && strcmp(argv[1], "provider") == 0) return provider_case(argv[2]);
    if (argc == 3 && strcmp(argv[1], "admission") == 0) return admission_case(argv[2]);
    if (argc == 5 && strcmp(argv[1], "generation") == 0)
        return generation_case(argv[2], argv[3], argv[4]);
    return 2;
}

#endif

static UmiStatus parent_health_probe(void *instance, UmiAiHealth *health)
{
    ParentFixture *f = instance;
    if (f->entered) return UMI_STATUS_INVALID_STATE;
    f->entered = 1;
    UmiStudioServices before;
    memcpy(&before, f->services, sizeof before);
    if (attempt_release(f) != UMI_STATUS_BUSY || f->destroy_calls != 0 ||
        memcmp(&before, f->services, sizeof before) != 0 ||
        UmiStudioAiPlatformCheckIdle(f->platform) != UMI_STATUS_BUSY ||
        UmiAiAuthorEngineServiceCheckIdle(umi_studio_ai_platform_authorengine(f->platform)) != UMI_STATUS_BUSY)
        f->failed = 1;
    health->available = 1;
    strcpy(health->message, "Parent health fixture");
    if (strcmp(f->phase, "malformed") == 0) memset(health->message, 'x', sizeof health->message);
    return strcmp(f->phase, "provider-error") == 0 ? UMI_STATUS_IO_ERROR : UMI_STATUS_OK;
}

static UmiStatus parent_scoped_health(void *context,
    const UmiAiRuntimeDescriptor *descriptor, UmiAiHealth *health)
{
    if (strcmp(descriptor->provider_id, "test.parent.health") != 0) {
        health->available = 0;
        return UMI_STATUS_OK;
    }
    return parent_health_probe(context, health);
}

static int health_case(const char *hook, const char *entry, const char *outcome)
{
    ParentFixture f = {0};
    int result = 1;
    REQUIRE(fixture_init(&f) == UMI_STATUS_OK);
    UmiAiRuntime *runtime = umi_studio_ai_platform_runtime(f.platform);
    UmiAiAuthorEngineService *service = umi_studio_ai_platform_authorengine(f.platform);
    UmiAiProvider provider = {sizeof provider, 1U, "test.parent.health", UMI_AI_PROVIDER_TEST,
        &f, unused_generate, parent_health_probe, provider_destroy};
    REQUIRE(umi_ai_provider_registry_add(&runtime->providers, &provider) == UMI_STATUS_OK);
    UmiAiRuntimeDescriptor descriptor = {0};
    strcpy(descriptor.runtime_id, "runtime.parent.health");
    strcpy(descriptor.provider_id, provider.provider_id);
    strcpy(descriptor.display_name, "Parent health fixture");
    strcpy(descriptor.model_id, "synthetic");
    descriptor.provider_kind = UMI_AI_PROVIDER_TEST;
    descriptor.transport = UMI_AI_RUNTIME_HTTP;
    descriptor.capabilities = UMI_AI_RUNTIME_CHAT;
    descriptor.context_tokens = 8192U; descriptor.max_output_tokens = 1024U;
    descriptor.configured = 1; descriptor.local = 1;
    REQUIRE(umi_ai_authorengine_service_register_runtime(service, &descriptor) == UMI_STATUS_OK);
    f.hook = hook; f.phase = outcome;
    size_t healthy = SIZE_MAX;
    UmiStatus status;
    if (strcmp(entry, "scoped") == 0)
        status = UmiAiAuthorEngineRefreshRuntimeHealth(service, 123U, parent_scoped_health, &f, &healthy);
    else {
        REQUIRE(strcmp(entry, "legacy") == 0);
        status = umi_ai_authorengine_service_refresh_health(service, 123U, &healthy);
    }
    UmiStatus expected = strcmp(outcome, "malformed") == 0 ? UMI_STATUS_INVALID_ARGUMENT :
        strcmp(entry, "scoped") == 0 && strcmp(outcome, "provider-error") == 0
        ? UMI_STATUS_IO_ERROR : UMI_STATUS_OK;
    REQUIRE(status == expected && f.entered && !f.failed && f.destroy_calls == 0);
    REQUIRE(UmiStudioAiPlatformCheckIdle(f.platform) == UMI_STATUS_OK);
    if (expected != UMI_STATUS_OK) REQUIRE(healthy == SIZE_MAX);
    f.hook = NULL;
    REQUIRE(UmiStudioServicesRelease(&f.services) == UMI_STATUS_OK && f.services == NULL);
    REQUIRE(f.destroy_calls == 1);
    result = 0;
cleanup:
    fixture_clear(&f); return result;
}

int main(int argc, char **argv)
{
    if (argc == 5 && strcmp(argv[1], "patch") == 0)
        return patch_case(argv[2], argv[3], strcmp(argv[4], "revert") == 0);
    if (argc == 3 && strcmp(argv[1], "provider") == 0) return provider_case(argv[2]);
    if (argc == 3 && strcmp(argv[1], "admission") == 0) return admission_case(argv[2]);
    if (argc == 5 && strcmp(argv[1], "generation") == 0)
        return generation_case(argv[2], argv[3], argv[4]);
    if (argc == 5 && strcmp(argv[1], "health") == 0)
        return health_case(argv[2], argv[3], argv[4]);
    return 2;
}
