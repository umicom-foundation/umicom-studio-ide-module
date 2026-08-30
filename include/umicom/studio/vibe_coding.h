/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/vibe_coding.h
 *
 * PURPOSE:
 *   Turn a plain-language development goal into a governed Studio coding
 *   session using the existing Framework context, RAG, provider and patch plan.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_VIBE_CODING_H
#define UMICOM_STUDIO_VIBE_CODING_H

#include "umicom/studio/coding_assistant.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum UmiStudioVibeCodingState {
    UMI_STUDIO_VIBE_CODING_IDLE = 0,
    UMI_STUDIO_VIBE_CODING_CONTEXT_READY = 1,
    UMI_STUDIO_VIBE_CODING_AWAITING_MODEL = 2,
    UMI_STUDIO_VIBE_CODING_AWAITING_REVIEW = 3,
    UMI_STUDIO_VIBE_CODING_APPLYING = 4,
    UMI_STUDIO_VIBE_CODING_VALIDATING = 5,
    UMI_STUDIO_VIBE_CODING_COMPLETE = 6,
    UMI_STUDIO_VIBE_CODING_FAILED = 7
} UmiStudioVibeCodingState;

typedef struct UmiStudioVibeCodingSession {
    uint32_t struct_size;
    uint32_t api_version;
    char request_id[UMI_AI_ID_CAPACITY];
    char goal[UMI_AI_TEXT_CAPACITY];
    char active_path[UMI_AI_TEXT_CAPACITY];
    char runtime_id[UMI_AI_ID_CAPACITY];
    char model_id[UMI_AI_ID_CAPACITY];
    char secret_reference[UMI_AI_TEXT_CAPACITY];
    UmiStudioVibeCodingState state;
    UmiAiCodingTaskPlan plan;
    int rag_enabled;
    int stream_responses;
    int requires_patch_approval;
    uint64_t revision;
} UmiStudioVibeCodingSession;

UmiStatus umi_studio_vibe_coding_prepare(
    UmiStudioAiPlatform *platform,
    const char *request_id,
    const char *goal,
    const char *active_path,
    UmiStudioVibeCodingSession *out_session);
const char *umi_studio_vibe_coding_state_text(
    UmiStudioVibeCodingState state);

#ifdef __cplusplus
}
#endif

#endif
