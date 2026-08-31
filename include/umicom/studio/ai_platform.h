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

/* BEGINNER NOTE:
 * Studio owns product composition only; provider-neutral AI and Helix mechanics remain inside Umicom Framework.
 */

#ifndef APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_AI_PLATFORM_H
#define APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_AI_PLATFORM_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UmiStudioAiPlatform UmiStudioAiPlatform;
typedef struct UmiStudioHelixAgentCentre UmiStudioHelixAgentCentre;

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

UmiStudioAiPlatformConfig umi_studio_ai_platform_config_default(void);
UmiStatus umi_studio_ai_platform_create(UmiStudioAiPlatform **out_platform);
UmiStatus umi_studio_ai_platform_create_configured(
    const UmiStudioAiPlatformConfig *config,
    UmiStudioAiPlatform **out_platform);
void umi_studio_ai_platform_destroy(UmiStudioAiPlatform *platform);
UmiAiRuntime *umi_studio_ai_platform_runtime(UmiStudioAiPlatform *platform);
/* A successful registration transfers adapter-instance cleanup to platform. */
UmiStatus umi_studio_ai_platform_register_provider(
    UmiStudioAiPlatform *platform,
    const UmiAiProvider *provider);
UmiHelixRuntime *umi_studio_ai_platform_helix(UmiStudioAiPlatform *platform);
UmiAiAuthorEngineService *umi_studio_ai_platform_authorengine(
    UmiStudioAiPlatform *platform);
UmiAiCodingAssistantService *umi_studio_ai_platform_coding_assistant(
    UmiStudioAiPlatform *platform);
UmiKnowledgeService *umi_studio_ai_platform_knowledge(
    UmiStudioAiPlatform *platform);
UmiStudioHelixAgentCentre *umi_studio_ai_platform_helix_agent_centre(
    UmiStudioAiPlatform *platform);
size_t umi_studio_ai_platform_knowledge_result_limit(
    const UmiStudioAiPlatform *platform);
const char *umi_studio_ai_platform_knowledge_archive_path(
    const UmiStudioAiPlatform *platform);
uint32_t umi_studio_ai_platform_coding_context_tokens(
    const UmiStudioAiPlatform *platform);
const char *umi_studio_ai_platform_default_provider(
    const UmiStudioAiPlatform *platform);
UmiStatus umi_studio_ai_platform_workbench_profile(
    const UmiStudioAiPlatform *platform,
    UmiStudioAiWorkbenchProfile *out_profile);
UmiStatus umi_studio_ai_platform_refresh_health(
    UmiStudioAiPlatform *platform,
    uint64_t timestamp_ns,
    size_t *out_healthy_runtimes);
UmiStatus umi_studio_ai_platform_begin_session(
    UmiStudioAiPlatform *platform,
    const char *session_id,
    const char *title,
    uint64_t timestamp_ns);
UmiStatus umi_studio_ai_platform_save_session(
    UmiStudioAiPlatform *platform,
    const char *session_id,
    const char *path);
UmiStatus umi_studio_ai_platform_snapshot(
    UmiStudioAiPlatform *platform,
    UmiAiAuthorEngineServiceSnapshot *out_snapshot);
UmiStatus umi_studio_ai_platform_compare_models(
    UmiStudioAiPlatform *platform,
    const char *prompt,
    const UmiAiModelTarget *targets,
    size_t target_count);
const UmiAiModelEnsembleReport *umi_studio_ai_platform_model_comparison(
    const UmiStudioAiPlatform *platform);

#ifdef __cplusplus
}
#endif

#endif
