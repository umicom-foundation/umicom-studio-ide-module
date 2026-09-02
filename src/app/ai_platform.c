/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/ai_platform.c
 *
 * PURPOSE:
 *   Create Studio's shared AI and Helix composition over Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * The built-in reference provider is deterministic and test-safe; real local and remote providers plug into the same Framework registry.
 */

#include "umicom/studio/ai_platform.h"
#include "umicom/studio/helix_agent_centre.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct UmiStudioAiPlatform {
    UmiAiRuntime ai;
    UmiHelixRuntime helix;
    UmiAiAuthorEngineService *authorengine;
    UmiAiCodingAssistantService *coding_assistant;
    UmiKnowledgeService *knowledge;
    UmiStudioHelixAgentCentre *helix_agent_centre;
    uint32_t coding_context_tokens;
    size_t knowledge_result_limit;
    char knowledge_archive_path[UMI_KNOWLEDGE_URI_CAPACITY];
    char default_provider[UMI_AI_ID_CAPACITY];
    UmiStudioAiWorkbenchProfile workbench_profile;
    UmiAiModelEnsembleReport model_comparison;
    uint64_t model_comparison_sequence;
};

/* Provide the copy text operation used by this module and its client applications. */
static int copy_text(char *destination, size_t capacity, const char *source)
{
    int written;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (destination == NULL || capacity == 0U || source == NULL) return 0;
    written = snprintf(destination, capacity, "%s", source);
    return written >= 0 && (size_t)written < capacity;
}

/*
 * Provide the studio local generate operation used by this module and its client
 * applications.
 */
static UmiStatus studio_local_generate(void *instance,
                                       const UmiAiRequest *request,
                                       UmiAiResponse *response)
{
    const UmiAiMessage *last;
    int written;
    (void)instance;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (request == NULL || response == NULL || request->message_count == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    last = &request->messages[request->message_count - 1U];
    umi_ai_response_init(response);
    written = snprintf(response->provider_id, sizeof(response->provider_id),
                       "%s", "studio.local");
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(response->provider_id)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    written = snprintf(response->model_id, sizeof(response->model_id),
                       "%s", request->model_id[0] != '\0'
                           ? request->model_id : "studio-reference");
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(response->model_id)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    written = snprintf(response->text, sizeof(response->text),
                       "Studio local reference provider received: %s", last->text);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(response->text)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    response->finish_reason = UMI_AI_FINISH_STOP;
    response->usage.input_tokens = umi_ai_estimate_tokens(last->text);
    response->usage.output_tokens = umi_ai_estimate_tokens(response->text);
    response->usage.total_tokens =
        response->usage.input_tokens + response->usage.output_tokens;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio local health operation used by this module and its client
 * applications.
 */
static UmiStatus studio_local_health(void *instance, UmiAiHealth *health)
{
    (void)instance;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (health == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)umi_ai_health_init(health);
    (void)snprintf(health->provider_id, sizeof(health->provider_id),
                   "%s", "studio.local");
    health->available = 1;
    (void)snprintf(health->message, sizeof(health->message),
                   "%s", "Reference provider is available");
    return UMI_STATUS_OK;
}

/*
 * Provide the studio ai platform config default operation used by this module and its
 * client applications.
 */
UmiStudioAiPlatformConfig umi_studio_ai_platform_config_default(void)
{
    UmiStudioAiPlatformConfig config;
    (void)memset(&config, 0, sizeof(config));
    (void)copy_text(config.authorengine_executable,
                    sizeof(config.authorengine_executable), "uaengine");
    (void)copy_text(config.workspace, sizeof(config.workspace), ".");
    config.context_tokens = 32768U;
    config.reserved_output_tokens = 2048U;
    config.coding_context_tokens = 16384U;
    config.maximum_patch_lines = 2000U;
    config.maximum_patch_files = UMI_AI_CODING_PATCH_FILE_MAX;
    config.allow_remote = 0;
    config.persist_sessions = 1;
    config.allow_patch_create = 1;
    config.allow_patch_delete = 0;
    config.require_patch_approval = 1;
    (void)copy_text(config.preferred_runtime_id,
                    sizeof(config.preferred_runtime_id),
                    "studio.local.reference");
    (void)copy_text(config.remote_provider_id,
                    sizeof(config.remote_provider_id),
                    "openai-compatible");
    config.remote_endpoint[0] = '\0';
    config.remote_model_id[0] = '\0';
    config.remote_secret_reference[0] = '\0';
    config.rag_enabled = 1;
    config.stream_responses = 1;
    (void)copy_text(config.knowledge_archive_path,
                    sizeof(config.knowledge_archive_path),
                    ".umicom/knowledge-centre.bin");
    config.knowledge_source_capacity = 1024U;
    config.knowledge_vector_capacity = 8192U;
    config.knowledge_chunk_bytes = 1200U;
    config.knowledge_overlap_bytes = 200U;
    config.knowledge_result_limit = 12U;
    config.knowledge_offline_only = 1;
    config.helix_maximum_attempts = 3U;
    config.helix_minimum_fitness = 0.80;
    config.helix_require_human_approval = 1;
    config.helix_allow_filesystem = 1;
    config.helix_allow_build = 1;
    config.helix_allow_test = 1;
    config.helix_allow_review = 1;
    config.helix_allow_source_control = 0;
    return config;
}

/*
 * Provide the create knowledge centre operation used by this module and its client
 * applications.
 */
static UmiStatus create_knowledge_centre(
    UmiStudioAiPlatform *platform,
    const UmiStudioAiPlatformConfig *config)
{
    UmiKnowledgeServiceConfig service_config =
        umi_knowledge_service_config_default();
    UmiKnowledgeCollection collection;
    UmiStatus status;
    service_config.source_capacity = config->knowledge_source_capacity;
    service_config.vector_capacity = config->knowledge_vector_capacity;
    service_config.chunk_policy.target_bytes = config->knowledge_chunk_bytes;
    service_config.chunk_policy.overlap_bytes = config->knowledge_overlap_bytes;
    service_config.chunk_policy.minimum_bytes =
        config->knowledge_chunk_bytes >= 96U ? 96U
                                             : config->knowledge_chunk_bytes;
    status = umi_knowledge_service_create(
        &service_config, &platform->knowledge);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_knowledge_collection_init(
            &collection, "project", "Active Project",
            "Project metadata, requirements and workspace documents.");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_knowledge_service_add_collection(
            platform->knowledge, &collection);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_knowledge_collection_init(
            &collection, "code", "Project Code",
            "Repository source code indexed with line provenance.");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_knowledge_service_add_collection(
            platform->knowledge, &collection);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_knowledge_collection_init(
            &collection, "documents", "Technical Documents",
            "Offline manuals, architecture records and technical references.");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_knowledge_service_add_collection(
            platform->knowledge, &collection);
    }
    return status;
}

/*
 * Provide the create helix agent centre operation used by this module and its client
 * applications.
 */
static UmiStatus create_helix_agent_centre(
    UmiStudioAiPlatform *platform,
    const UmiStudioAiPlatformConfig *config)
{
    UmiStudioHelixAgentCentreConfig centre_config =
        umi_studio_helix_agent_centre_config_default();
    centre_config.maximum_attempts = config->helix_maximum_attempts;
    centre_config.minimum_fitness = config->helix_minimum_fitness;
    centre_config.require_human_approval = config->helix_require_human_approval;
    centre_config.allow_filesystem = config->helix_allow_filesystem;
    centre_config.allow_build = config->helix_allow_build;
    centre_config.allow_test = config->helix_allow_test;
    centre_config.allow_review = config->helix_allow_review;
    centre_config.allow_source_control = config->helix_allow_source_control;
    return umi_studio_helix_agent_centre_create(
        &centre_config, &platform->helix_agent_centre);
}

/*
 * Provide the register catalogue runtime operation used by this module and its client
 * applications.
 */
static UmiStatus register_catalogue_runtime(
    UmiStudioAiPlatform *platform,
    const char *runtime_id,
    const char *provider_id,
    const char *display_name,
    const char *model_id,
    const char *endpoint,
    UmiAiProviderKind provider_kind,
    UmiAiRuntimeTransport transport,
    uint32_t context_tokens,
    uint32_t output_tokens,
    int configured,
    int local,
    int available,
    const char *health_message)
{
    UmiAiRuntimeDescriptor descriptor;
    (void)memset(&descriptor, 0, sizeof(descriptor));
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (!copy_text(descriptor.runtime_id, sizeof(descriptor.runtime_id),
                   runtime_id) ||
        !copy_text(descriptor.provider_id, sizeof(descriptor.provider_id),
                   provider_id) ||
        !copy_text(descriptor.display_name, sizeof(descriptor.display_name),
                   display_name) ||
        !copy_text(descriptor.model_id, sizeof(descriptor.model_id), model_id) ||
        !copy_text(descriptor.endpoint, sizeof(descriptor.endpoint), endpoint) ||
        !copy_text(descriptor.health.provider_id,
                   sizeof(descriptor.health.provider_id), provider_id) ||
        !copy_text(descriptor.health.message,
                   sizeof(descriptor.health.message), health_message)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    descriptor.provider_kind = provider_kind;
    descriptor.transport = transport;
    descriptor.capabilities = UMI_AI_RUNTIME_CHAT |
                              UMI_AI_RUNTIME_COMPLETION |
                              UMI_AI_RUNTIME_STREAMING;
    descriptor.context_tokens = context_tokens;
    descriptor.max_output_tokens = output_tokens;
    descriptor.configured = configured;
    descriptor.local = local;
    descriptor.health.available = available;
    return umi_ai_authorengine_service_register_runtime(
        platform->authorengine, &descriptor);
}

/* Provide the register context operation used by this module and its client applications. */
static UmiStatus register_context(UmiStudioAiPlatform *platform,
                                  const char *source_id,
                                  const char *label,
                                  const char *uri,
                                  UmiAiContextSourceKind kind,
                                  UmiAiDataClassification classification,
                                  uint32_t tokens,
                                  uint32_t priority,
                                  int mandatory,
                                  int enabled)
{
    UmiAiContextSource source;
    (void)memset(&source, 0, sizeof(source));
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (!copy_text(source.source_id, sizeof(source.source_id), source_id) ||
        !copy_text(source.label, sizeof(source.label), label) ||
        !copy_text(source.uri, sizeof(source.uri), uri)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    source.kind = kind;
    source.classification = classification;
    source.estimated_tokens = tokens;
    source.priority = priority;
    source.mandatory = mandatory;
    source.enabled = enabled;
    return umi_ai_context_broker_upsert(
        umi_ai_authorengine_service_context(platform->authorengine), &source);
}

/*
 * Provide the register coding file operation used by this module and its client
 * applications.
 */
static UmiStatus register_coding_file(UmiStudioAiPlatform *platform,
                                      const char *path,
                                      const char *language_id,
                                      const char *summary,
                                      uint32_t tokens,
                                      uint32_t priority,
                                      int active)
{
    UmiAiCodingContextFile file;
    (void)memset(&file, 0, sizeof(file));
    /* Apply this branch only when its contract condition is satisfied. */
    if (!copy_text(file.path, sizeof(file.path), path) ||
        !copy_text(file.language_id, sizeof(file.language_id), language_id) ||
        !copy_text(file.summary, sizeof(file.summary), summary)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    file.classification = UMI_AI_DATA_INTERNAL;
    file.estimated_tokens = tokens;
    file.priority = priority;
    file.active = active;
    file.enabled = 1;
    return umi_ai_coding_context_upsert(
        umi_ai_coding_assistant_context(platform->coding_assistant), &file);
}

/*
 * Provide the studio ai platform create configured operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_ai_platform_create_configured(
    const UmiStudioAiPlatformConfig *config,
    UmiStudioAiPlatform **out_platform)
{
    UmiStudioAiPlatform *platform;
    UmiAiAuthorEngineConfig authorengine_config;
    UmiAiPolicy provider_policy;
    UmiAiPrivacyPolicy privacy_policy;
    UmiAiProvider provider;
    UmiStatus status;
    int remote_requested;

    remote_requested = config != NULL && config->allow_remote &&
        config->remote_endpoint[0] != '\0';

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (config == NULL || out_platform == NULL ||
        config->authorengine_executable[0] == '\0' ||
        config->workspace[0] == '\0' || config->context_tokens == 0U ||
        config->reserved_output_tokens >= config->context_tokens ||
        config->coding_context_tokens == 0U ||
        config->maximum_patch_lines == 0U ||
        config->maximum_patch_files == 0U ||
        config->maximum_patch_files > UMI_AI_CODING_PATCH_FILE_MAX ||
        config->preferred_runtime_id[0] == '\0' ||
        config->knowledge_archive_path[0] == '\0' ||
        config->knowledge_source_capacity == 0U ||
        config->knowledge_vector_capacity == 0U ||
        config->knowledge_chunk_bytes == 0U ||
        config->knowledge_chunk_bytes >= UMI_KNOWLEDGE_TEXT_CAPACITY ||
        config->knowledge_overlap_bytes >= config->knowledge_chunk_bytes ||
        config->knowledge_result_limit == 0U ||
        config->knowledge_result_limit > UMI_KNOWLEDGE_QUERY_RESULT_MAX ||
        config->helix_maximum_attempts == 0U ||
        config->helix_minimum_fitness < 0.0 ||
        config->helix_minimum_fitness > 1.0 ||
        (remote_requested &&
         (config->remote_provider_id[0] == '\0' ||
          config->remote_model_id[0] == '\0' ||
          config->remote_secret_reference[0] == '\0' ||
          strstr(config->remote_secret_reference, "://") == NULL))) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_platform = NULL;
    platform = (UmiStudioAiPlatform *)calloc(1U, sizeof(*platform));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    platform->coding_context_tokens = config->coding_context_tokens;
    platform->knowledge_result_limit = config->knowledge_result_limit;
    /* Apply this branch only when its contract condition is satisfied. */
    if (!copy_text(platform->knowledge_archive_path,
                   sizeof(platform->knowledge_archive_path),
                   config->knowledge_archive_path)) {
        free(platform);
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    platform->workbench_profile.struct_size =
        (uint32_t)sizeof(platform->workbench_profile);
    platform->workbench_profile.api_version = 1U;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (!copy_text(platform->workbench_profile.preferred_runtime_id,
                   sizeof(platform->workbench_profile.preferred_runtime_id),
                   config->preferred_runtime_id) ||
        !copy_text(platform->workbench_profile.remote_provider_id,
                   sizeof(platform->workbench_profile.remote_provider_id),
                   config->remote_provider_id) ||
        !copy_text(platform->workbench_profile.remote_endpoint,
                   sizeof(platform->workbench_profile.remote_endpoint),
                   config->remote_endpoint) ||
        !copy_text(platform->workbench_profile.remote_model_id,
                   sizeof(platform->workbench_profile.remote_model_id),
                   config->remote_model_id) ||
        !copy_text(platform->workbench_profile.remote_secret_reference,
                   sizeof(platform->workbench_profile.remote_secret_reference),
                   config->remote_secret_reference)) {
        free(platform);
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    platform->workbench_profile.remote_configured =
        config->allow_remote && config->remote_endpoint[0] != '\0' &&
        config->remote_provider_id[0] != '\0' &&
        config->remote_model_id[0] != '\0' &&
        config->remote_secret_reference[0] != '\0';
    platform->workbench_profile.rag_enabled = config->rag_enabled != 0;
    platform->workbench_profile.stream_responses =
        config->stream_responses != 0;
    platform->workbench_profile.require_patch_approval =
        config->require_patch_approval != 0;

    umi_ai_runtime_init(&platform->ai);
    status = umi_ai_model_ensemble_report_initialize(
        &platform->model_comparison);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        free(platform);
        return status;
    }
    platform->ai.policy.allow_remote = config->allow_remote != 0;
    platform->ai.policy.allow_tools = 1;
    platform->ai.policy.require_tool_approval = 1;
    umi_helix_runtime_init(&platform->helix, "studio.helix");
    status = umi_helix_runtime_register_default_agents(&platform->helix,
                                                       "studio.local");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        free(platform);
        return status;
    }

    (void)memset(&provider, 0, sizeof(provider));
    provider.structure_size = (uint32_t)sizeof(provider);
    provider.abi_version = 1U;
    provider.provider_id = "studio.local";
    provider.kind = UMI_AI_PROVIDER_TEST;
    provider.generate = studio_local_generate;
    provider.health = studio_local_health;

    status = umi_ai_provider_registry_add(&platform->ai.providers, &provider);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        free(platform);
        return status;
    }
    (void)snprintf(platform->default_provider,
                   sizeof(platform->default_provider),
                   "%s", provider.provider_id);

    (void)memset(&authorengine_config, 0, sizeof(authorengine_config));
    /* Apply this branch only when its contract condition is satisfied. */
    if (!copy_text(authorengine_config.executable,
                   sizeof(authorengine_config.executable),
                   config->authorengine_executable) ||
        !copy_text(authorengine_config.workspace,
                   sizeof(authorengine_config.workspace), config->workspace) ||
        !copy_text(authorengine_config.provider,
                   sizeof(authorengine_config.provider), "authorengine.local")) {
        free(platform);
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    status = umi_ai_authorengine_service_create(
        &platform->ai, &authorengine_config, &platform->authorengine);
    provider_policy = platform->ai.policy;
    provider_policy.allow_remote = config->allow_remote != 0;
    privacy_policy = umi_ai_privacy_policy_default();
    privacy_policy.persist_sessions = config->persist_sessions != 0;
    privacy_policy.persist_prompt_text = config->persist_sessions != 0;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ai_authorengine_service_set_policy(
            platform->authorengine, &provider_policy, &privacy_policy,
            config->context_tokens, config->reserved_output_tokens);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = register_catalogue_runtime(
            platform, "studio.local.reference", "studio.local",
            "Studio Local Reference", "studio-reference", "in-process://studio",
            UMI_AI_PROVIDER_TEST, UMI_AI_RUNTIME_IN_PROCESS, 8192U, 1024U,
            1, 1, 1, "Reference provider is available");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = register_catalogue_runtime(
            platform, "authorengine.local.chat", "authorengine.local",
            "AuthorEngine Local", "authorengine-local", config->authorengine_executable,
            UMI_AI_PROVIDER_AUTHOR_ENGINE, UMI_AI_RUNTIME_PROCESS,
            config->context_tokens, config->reserved_output_tokens,
            1, 1, 0, "AuthorEngine health probe pending");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = register_catalogue_runtime(
            platform, "authorengine.remote.chat", "authorengine.remote",
            "AuthorEngine Remote Gateway", "authorengine-remote",
            "process://authorengine/remote", UMI_AI_PROVIDER_REMOTE,
            UMI_AI_RUNTIME_PROCESS, config->context_tokens,
            config->reserved_output_tokens, config->allow_remote != 0, 0, 0,
            config->allow_remote ? "Remote health probe pending"
                                 : "Remote providers disabled by policy");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK &&
        platform->workbench_profile.remote_configured) {
        status = register_catalogue_runtime(
            platform, config->preferred_runtime_id,
            config->remote_provider_id, "Configured Remote Coding Provider",
            config->remote_model_id, config->remote_endpoint,
            UMI_AI_PROVIDER_REMOTE, UMI_AI_RUNTIME_HTTP,
            config->context_tokens, config->reserved_output_tokens,
            1, 0, 0, "Remote provider health probe pending");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = register_context(
            platform, "studio.workspace", "Active workspace",
            config->workspace, UMI_AI_CONTEXT_WORKSPACE, UMI_AI_DATA_INTERNAL,
            2048U, 100U, 1, 1);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = register_context(
            platform, "studio.project", "Active project metadata",
            "workspace://project", UMI_AI_CONTEXT_PROJECT,
            UMI_AI_DATA_INTERNAL, 4096U, 80U, 0, 1);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = register_context(
            platform, "studio.diagnostics", "Current diagnostics",
            "workspace://diagnostics", UMI_AI_CONTEXT_DIAGNOSTICS,
            UMI_AI_DATA_CONFIDENTIAL, 1024U, 60U, 0, 0);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ai_authorengine_service_begin_session(
            platform->authorengine, "studio.session.default", "studio.local",
            "studio-reference", "studio.workspace", "Studio AI Workspace", 0U);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        UmiAiCodingPatchPolicy patch_policy =
            umi_ai_coding_patch_policy_default();
        status = umi_ai_coding_assistant_create(
            platform->authorengine, &platform->coding_assistant);
        patch_policy.maximum_files = config->maximum_patch_files;
        patch_policy.maximum_changed_lines = config->maximum_patch_lines;
        patch_policy.allow_create = config->allow_patch_create != 0;
        patch_policy.allow_delete = config->allow_patch_delete != 0;
        patch_policy.require_approval = config->require_patch_approval != 0;
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            status = umi_ai_coding_assistant_set_patch_policy(
                platform->coding_assistant, &patch_policy);
        }
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = register_coding_file(
        platform, "CMakeLists.txt", "cmake", "Studio root build composition",
        900U, 80U, 0);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = register_coding_file(
        platform, "applications/studio/CMakeLists.txt", "cmake",
        "Studio targets and focused tests", 1800U, 90U, 0);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = register_coding_file(
        platform, "applications/studio/src/app/ai_platform.c", "c23",
        "Active AI and AuthorEngine product composition", 2200U, 100U, 1);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = register_coding_file(
        platform, "framework/include/umicom/ai/ai.h", "c23",
        "Framework AI public aggregate", 500U, 70U, 0);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = create_knowledge_centre(platform, config);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = create_helix_agent_centre(platform, config);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_ai_platform_destroy(platform);
        return status;
    }
    *out_platform = platform;
    return UMI_STATUS_OK;
}

/*
 * Initialise studio ai platform from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_ai_platform_create(UmiStudioAiPlatform **out_platform)
{
    UmiStudioAiPlatformConfig config = umi_studio_ai_platform_config_default();
    return umi_studio_ai_platform_create_configured(&config, out_platform);
}

/*
 * Release or reset state held by studio ai platform so the same storage can be reused
 * safely.
 */
void umi_studio_ai_platform_destroy(UmiStudioAiPlatform *platform)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL) return;
    umi_studio_helix_agent_centre_destroy(platform->helix_agent_centre);
    umi_knowledge_service_destroy(platform->knowledge);
    umi_ai_coding_assistant_destroy(platform->coding_assistant);
    umi_ai_authorengine_service_destroy(platform->authorengine);
    /* Release registered adapter instances after their consumers are gone. */
    umi_ai_runtime_destroy(&platform->ai);
    free(platform);
}

/*
 * Provide the studio ai platform runtime operation used by this module and its client
 * applications.
 */
UmiAiRuntime *umi_studio_ai_platform_runtime(UmiStudioAiPlatform *platform)
{
    return platform != NULL ? &platform->ai : NULL;
}

/*
 * Provide the studio ai platform register provider operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_ai_platform_register_provider(
    UmiStudioAiPlatform *platform,
    const UmiAiProvider *provider)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL || provider == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    /* Adapters register once here; every request still passes runtime policy. */
    return umi_ai_provider_registry_add(&platform->ai.providers, provider);
}

/*
 * Provide the studio ai platform helix operation used by this module and its client
 * applications.
 */
UmiHelixRuntime *umi_studio_ai_platform_helix(UmiStudioAiPlatform *platform)
{
    return platform != NULL ? &platform->helix : NULL;
}

/*
 * Provide the studio ai platform authorengine operation used by this module and its client
 * applications.
 */
UmiAiAuthorEngineService *umi_studio_ai_platform_authorengine(
    UmiStudioAiPlatform *platform)
{
    return platform != NULL ? platform->authorengine : NULL;
}

/*
 * Provide the studio ai platform coding assistant operation used by this module and its
 * client applications.
 */
UmiAiCodingAssistantService *umi_studio_ai_platform_coding_assistant(
    UmiStudioAiPlatform *platform)
{
    return platform != NULL ? platform->coding_assistant : NULL;
}

/*
 * Provide the studio ai platform knowledge operation used by this module and its client
 * applications.
 */
UmiKnowledgeService *umi_studio_ai_platform_knowledge(
    UmiStudioAiPlatform *platform)
{
    return platform != NULL ? platform->knowledge : NULL;
}

/*
 * Provide the studio ai platform helix agent centre operation used by this module and its
 * client applications.
 */
UmiStudioHelixAgentCentre *umi_studio_ai_platform_helix_agent_centre(
    UmiStudioAiPlatform *platform)
{
    return platform != NULL ? platform->helix_agent_centre : NULL;
}

/*
 * Provide the studio ai platform knowledge result limit operation used by this module and
 * its client applications.
 */
size_t umi_studio_ai_platform_knowledge_result_limit(
    const UmiStudioAiPlatform *platform)
{
    return platform != NULL ? platform->knowledge_result_limit : 0U;
}

/*
 * Provide the studio ai platform knowledge archive path operation used by this module and
 * its client applications.
 */
const char *umi_studio_ai_platform_knowledge_archive_path(
    const UmiStudioAiPlatform *platform)
{
    return platform != NULL ? platform->knowledge_archive_path : NULL;
}

/*
 * Provide the studio ai platform coding context tokens operation used by this module and
 * its client applications.
 */
uint32_t umi_studio_ai_platform_coding_context_tokens(
    const UmiStudioAiPlatform *platform)
{
    return platform != NULL ? platform->coding_context_tokens : 0U;
}

/*
 * Provide the studio ai platform default provider operation used by this module and its
 * client applications.
 */
const char *umi_studio_ai_platform_default_provider(
    const UmiStudioAiPlatform *platform)
{
    return platform != NULL ? platform->default_provider : NULL;
}

/*
 * Provide the studio ai platform workbench profile operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_ai_platform_workbench_profile(
    const UmiStudioAiPlatform *platform,
    UmiStudioAiWorkbenchProfile *out_profile)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL || out_profile == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_profile = platform->workbench_profile;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio ai platform refresh health operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_ai_platform_refresh_health(
    UmiStudioAiPlatform *platform,
    uint64_t timestamp_ns,
    size_t *out_healthy_runtimes)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_ai_authorengine_service_refresh_health(
        platform->authorengine, timestamp_ns, out_healthy_runtimes);
}

/*
 * Provide the studio ai platform begin session operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_ai_platform_begin_session(
    UmiStudioAiPlatform *platform,
    const char *session_id,
    const char *title,
    uint64_t timestamp_ns)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_ai_authorengine_service_begin_session(
        platform->authorengine, session_id, "studio.local", "studio-reference",
        "studio.workspace", title, timestamp_ns);
}

/*
 * Provide the studio ai platform save session operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_ai_platform_save_session(
    UmiStudioAiPlatform *platform,
    const char *session_id,
    const char *path)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_ai_authorengine_service_save_session(
        platform->authorengine, session_id, path);
}

/*
 * Provide the studio ai platform snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_ai_platform_snapshot(
    UmiStudioAiPlatform *platform,
    UmiAiAuthorEngineServiceSnapshot *out_snapshot)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_ai_authorengine_service_snapshot(
        platform->authorengine, out_snapshot);
}

/*
 * Provide the studio ai platform compare models operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_ai_platform_compare_models(
    UmiStudioAiPlatform *platform,
    const char *prompt,
    const UmiAiModelTarget *targets,
    size_t target_count)
{
    UmiAiRequest request;
    UmiAiMessage message;
    UmiStatus status;
    int written;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL || prompt == NULL || prompt[0] == '\0' ||
        targets == NULL || target_count == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Studio supplies only prompt composition; Framework owns routing policy. */
    umi_ai_request_init(&request);
    ++platform->model_comparison_sequence;
    written = snprintf(request.request_id, sizeof(request.request_id),
                       "studio.model-comparison.%" PRIu64,
                       platform->model_comparison_sequence);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(request.request_id)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    status = umi_ai_message_set(&message, UMI_AI_ROLE_USER, "user", prompt);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ai_request_add_message(&request, &message);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ai_model_ensemble_query(
            &platform->ai, &request, targets, target_count,
            &platform->model_comparison);
    }
    return status;
}

/*
 * Provide the studio ai platform model comparison operation used by this module and its
 * client applications.
 */
const UmiAiModelEnsembleReport *umi_studio_ai_platform_model_comparison(
    const UmiStudioAiPlatform *platform)
{
    return platform != NULL ? &platform->model_comparison : NULL;
}
