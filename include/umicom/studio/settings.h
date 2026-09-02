/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/settings.h
 *
 * PURPOSE:
 *   Define the Studio-owned settings schema and provide narrow helpers for
 *   creating, loading, saving and locating the typed Framework settings store.
 *   UI code and feature modules consume this contract rather than parsing JSON
 *   or environment variables directly.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SETTINGS_H
#define UMICOM_STUDIO_SETTINGS_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_SETTINGS_DEFAULT_PATH "config/studio.settings"
#define UMI_STUDIO_SETTING_COUNT 45U

#define UMI_STUDIO_SETTING_UI_THEME "studio.ui.theme"
#define UMI_STUDIO_SETTING_EDITOR_TAB_WIDTH "studio.editor.tab_width"
#define UMI_STUDIO_SETTING_EDITOR_INSERT_SPACES \
    "studio.editor.insert_spaces"
#define UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION \
    "studio.workspace.restore_session"
#define UMI_STUDIO_SETTING_BUILD_PARALLEL_JOBS \
    "studio.build.parallel_jobs"
#define UMI_STUDIO_SETTING_AI_ENABLED "studio.ai.enabled"
#define UMI_STUDIO_SETTING_AUTHORENGINE_EXECUTABLE \
    "studio.ai.authorengine.executable"
#define UMI_STUDIO_SETTING_AUTHORENGINE_WORKSPACE \
    "studio.ai.authorengine.workspace"
#define UMI_STUDIO_SETTING_AI_CONTEXT_TOKENS \
    "studio.ai.context_tokens"
#define UMI_STUDIO_SETTING_AI_RESERVED_OUTPUT_TOKENS \
    "studio.ai.reserved_output_tokens"
#define UMI_STUDIO_SETTING_AI_ALLOW_REMOTE \
    "studio.ai.allow_remote"
#define UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS \
    "studio.ai.persist_sessions"
#define UMI_STUDIO_SETTING_AI_CODING_CONTEXT_TOKENS \
    "studio.ai.coding.context_tokens"
#define UMI_STUDIO_SETTING_AI_CODING_MAX_PATCH_FILES \
    "studio.ai.coding.maximum_patch_files"
#define UMI_STUDIO_SETTING_AI_CODING_MAX_PATCH_LINES \
    "studio.ai.coding.maximum_patch_lines"
#define UMI_STUDIO_SETTING_AI_CODING_ALLOW_CREATE \
    "studio.ai.coding.allow_file_creation"
#define UMI_STUDIO_SETTING_AI_CODING_ALLOW_DELETE \
    "studio.ai.coding.allow_file_deletion"
#define UMI_STUDIO_SETTING_AI_CODING_REQUIRE_APPROVAL \
    "studio.ai.coding.require_patch_approval"
#define UMI_STUDIO_SETTING_AI_PREFERRED_RUNTIME \
    "studio.ai.coding.preferred_runtime"
#define UMI_STUDIO_SETTING_AI_REMOTE_PROVIDER \
    "studio.ai.remote.provider"
#define UMI_STUDIO_SETTING_AI_REMOTE_ENDPOINT \
    "studio.ai.remote.endpoint"
#define UMI_STUDIO_SETTING_AI_REMOTE_MODEL \
    "studio.ai.remote.model"
#define UMI_STUDIO_SETTING_AI_REMOTE_SECRET_REFERENCE \
    "studio.ai.remote.secret_reference"
#define UMI_STUDIO_SETTING_AI_RAG_ENABLED \
    "studio.ai.coding.rag_enabled"
#define UMI_STUDIO_SETTING_AI_STREAM_RESPONSES \
    "studio.ai.coding.stream_responses"
#define UMI_STUDIO_SETTING_KNOWLEDGE_ARCHIVE_PATH \
    "studio.knowledge.archive_path"
#define UMI_STUDIO_SETTING_KNOWLEDGE_SOURCE_CAPACITY \
    "studio.knowledge.source_capacity"
#define UMI_STUDIO_SETTING_KNOWLEDGE_VECTOR_CAPACITY \
    "studio.knowledge.vector_capacity"
#define UMI_STUDIO_SETTING_KNOWLEDGE_CHUNK_BYTES \
    "studio.knowledge.chunk_bytes"
#define UMI_STUDIO_SETTING_KNOWLEDGE_OVERLAP_BYTES \
    "studio.knowledge.overlap_bytes"
#define UMI_STUDIO_SETTING_KNOWLEDGE_RESULT_LIMIT \
    "studio.knowledge.result_limit"
#define UMI_STUDIO_SETTING_KNOWLEDGE_OFFLINE_ONLY \
    "studio.knowledge.offline_only"
#define UMI_STUDIO_SETTING_HELIX_MAXIMUM_ATTEMPTS \
    "studio.helix.maximum_attempts"
#define UMI_STUDIO_SETTING_HELIX_REQUIRE_APPROVAL \
    "studio.helix.require_human_approval"
#define UMI_STUDIO_SETTING_HELIX_ALLOW_FILESYSTEM \
    "studio.helix.allow_filesystem"
#define UMI_STUDIO_SETTING_HELIX_ALLOW_BUILD \
    "studio.helix.allow_build"
#define UMI_STUDIO_SETTING_HELIX_ALLOW_SOURCE_CONTROL \
    "studio.helix.allow_source_control"
#define UMI_STUDIO_SETTING_DIAGNOSTIC_CAPACITY \
    "studio.diagnostics.capacity"
#define UMI_STUDIO_SETTING_AUTO_SAVE "studio.files.auto_save"
#define UMI_STUDIO_SETTING_AUTO_SAVE_SECONDS \
    "studio.files.auto_save_seconds"
#define UMI_STUDIO_SETTING_VISUAL_BUILDER_GRID_SIZE \
    "studio.visual_builder.grid_size"
#define UMI_STUDIO_SETTING_VISUAL_BUILDER_SNAP \
    "studio.visual_builder.snap_to_grid"
#define UMI_STUDIO_SETTING_VISUAL_BUILDER_PREVIEW_PROFILE \
    "studio.visual_builder.preview_profile"
#define UMI_STUDIO_SETTING_VISUAL_BUILDER_SHOW_SOURCE \
    "studio.visual_builder.show_generated_source"
#define UMI_STUDIO_SETTING_VISUAL_BUILDER_AUTO_PREVIEW \
    "studio.visual_builder.auto_refresh_preview"

/**
 * Initialise studio settings from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_settings_create(UmiSettings **out_settings);

/**
 * Provide the studio settings load if present operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_settings_load_if_present(
    UmiSettings *settings,
    const char *path,
    int *out_loaded
);

/**
 * Write studio settings in its stable representation and report capacity or input failures
 * to the caller.
 */
UmiStatus umi_studio_settings_save(
    const UmiSettings *settings,
    const char *path
);

/**
 * Provide the studio settings default path operation used by this module and its client
 * applications.
 */
const char *umi_studio_settings_default_path(void);

#ifdef __cplusplus
}
#endif

#endif
