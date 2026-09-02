/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/ai_platform.h
 *
 * PURPOSE:
 *   Own the shared Framework AI runtime and Helix runtime used by Studio workflows.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio owns product composition only; provider-neutral AI and Helix mechanics remain inside Umicom Framework.
 */

#ifndef APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_AI_PLATFORM_H
#define APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_AI_PLATFORM_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio ai platform data shared with callers of this public contract.
 */
typedef struct UmiStudioAiPlatform UmiStudioAiPlatform;
/**
 * Represent the studio helix agent centre data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioHelixAgentCentre UmiStudioHelixAgentCentre;

/**
 * Represent the studio ai platform config data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioAiPlatformConfig {
    char authorengine_executable[UMI_AI_TEXT_CAPACITY];
    char workspace[UMI_AI_TEXT_CAPACITY];
    uint32_t context_tokens;
    uint32_t reserved_output_tokens;
    uint32_t coding_context_tokens;
    uint32_t maximum_patch_lines;
    size_t maximum_patch_files;
    int allow_remote;
    int persist_sessions;
    int allow_patch_create;
    int allow_patch_delete;
    int require_patch_approval;
    char preferred_runtime_id[UMI_AI_ID_CAPACITY];
    char remote_provider_id[UMI_AI_ID_CAPACITY];
    char remote_endpoint[UMI_AI_TEXT_CAPACITY];
    char remote_model_id[UMI_AI_ID_CAPACITY];
    char remote_secret_reference[UMI_AI_TEXT_CAPACITY];
    int rag_enabled;
    int stream_responses;
    char knowledge_archive_path[UMI_KNOWLEDGE_URI_CAPACITY];
    size_t knowledge_source_capacity;
    size_t knowledge_vector_capacity;
    size_t knowledge_chunk_bytes;
    size_t knowledge_overlap_bytes;
    size_t knowledge_result_limit;
    int knowledge_offline_only;
    uint32_t helix_maximum_attempts;
    double helix_minimum_fitness;
    int helix_require_human_approval;
    int helix_allow_filesystem;
    int helix_allow_build;
    int helix_allow_test;
    int helix_allow_review;
    int helix_allow_source_control;
} UmiStudioAiPlatformConfig;

/**
 * Represent the studio ai workbench profile data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioAiWorkbenchProfile {
    uint32_t struct_size;
    uint32_t api_version;
    char preferred_runtime_id[UMI_AI_ID_CAPACITY];
    char remote_provider_id[UMI_AI_ID_CAPACITY];
    char remote_endpoint[UMI_AI_TEXT_CAPACITY];
    char remote_model_id[UMI_AI_ID_CAPACITY];
    char remote_secret_reference[UMI_AI_TEXT_CAPACITY];
    int remote_configured;
    int rag_enabled;
    int stream_responses;
    int require_patch_approval;
} UmiStudioAiWorkbenchProfile;

/**
 * Provide the studio ai platform config default operation used by this module and its
 * client applications.
 */
UmiStudioAiPlatformConfig umi_studio_ai_platform_config_default(void);
/**
 * Initialise studio ai platform from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_ai_platform_create(UmiStudioAiPlatform **out_platform);
/**
 * Provide the studio ai platform create configured operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_ai_platform_create_configured(
    const UmiStudioAiPlatformConfig *config,
    UmiStudioAiPlatform **out_platform);
/**
 * Release or reset state held by studio ai platform so the same storage can be reused
 * safely.
 */
void umi_studio_ai_platform_destroy(UmiStudioAiPlatform *platform);
/**
 * Provide the studio ai platform runtime operation used by this module and its client
 * applications.
 */
UmiAiRuntime *umi_studio_ai_platform_runtime(UmiStudioAiPlatform *platform);
/* A successful registration transfers adapter-instance cleanup to platform. */
UmiStatus umi_studio_ai_platform_register_provider(
    UmiStudioAiPlatform *platform,
    const UmiAiProvider *provider);
/**
 * Provide the studio ai platform helix operation used by this module and its client
 * applications.
 */
UmiHelixRuntime *umi_studio_ai_platform_helix(UmiStudioAiPlatform *platform);
/**
 * Provide the studio ai platform authorengine operation used by this module and its client
 * applications.
 */
UmiAiAuthorEngineService *umi_studio_ai_platform_authorengine(
    UmiStudioAiPlatform *platform);
/**
 * Provide the studio ai platform coding assistant operation used by this module and its
 * client applications.
 */
UmiAiCodingAssistantService *umi_studio_ai_platform_coding_assistant(
    UmiStudioAiPlatform *platform);
/**
 * Provide the studio ai platform knowledge operation used by this module and its client
 * applications.
 */
UmiKnowledgeService *umi_studio_ai_platform_knowledge(
    UmiStudioAiPlatform *platform);
/**
 * Provide the studio ai platform helix agent centre operation used by this module and its
 * client applications.
 */
UmiStudioHelixAgentCentre *umi_studio_ai_platform_helix_agent_centre(
    UmiStudioAiPlatform *platform);
/**
 * Provide the studio ai platform knowledge result limit operation used by this module and
 * its client applications.
 */
size_t umi_studio_ai_platform_knowledge_result_limit(
    const UmiStudioAiPlatform *platform);
/**
 * Provide the studio ai platform knowledge archive path operation used by this module and
 * its client applications.
 */
const char *umi_studio_ai_platform_knowledge_archive_path(
    const UmiStudioAiPlatform *platform);
/**
 * Provide the studio ai platform coding context tokens operation used by this module and
 * its client applications.
 */
uint32_t umi_studio_ai_platform_coding_context_tokens(
    const UmiStudioAiPlatform *platform);
/**
 * Provide the studio ai platform default provider operation used by this module and its
 * client applications.
 */
const char *umi_studio_ai_platform_default_provider(
    const UmiStudioAiPlatform *platform);
/**
 * Provide the studio ai platform workbench profile operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_ai_platform_workbench_profile(
    const UmiStudioAiPlatform *platform,
    UmiStudioAiWorkbenchProfile *out_profile);
/**
 * Provide the studio ai platform refresh health operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_ai_platform_refresh_health(
    UmiStudioAiPlatform *platform,
    uint64_t timestamp_ns,
    size_t *out_healthy_runtimes);
/**
 * Provide the studio ai platform begin session operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_ai_platform_begin_session(
    UmiStudioAiPlatform *platform,
    const char *session_id,
    const char *title,
    uint64_t timestamp_ns);
/**
 * Provide the studio ai platform save session operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_ai_platform_save_session(
    UmiStudioAiPlatform *platform,
    const char *session_id,
    const char *path);
/**
 * Provide the studio ai platform snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_ai_platform_snapshot(
    UmiStudioAiPlatform *platform,
    UmiAiAuthorEngineServiceSnapshot *out_snapshot);
/**
 * Provide the studio ai platform compare models operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_ai_platform_compare_models(
    UmiStudioAiPlatform *platform,
    const char *prompt,
    const UmiAiModelTarget *targets,
    size_t target_count);
/**
 * Provide the studio ai platform model comparison operation used by this module and its
 * client applications.
 */
const UmiAiModelEnsembleReport *umi_studio_ai_platform_model_comparison(
    const UmiStudioAiPlatform *platform);

#ifdef __cplusplus
}
#endif

#endif
