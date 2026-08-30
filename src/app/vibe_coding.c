/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/vibe_coding.c
 *
 * PURPOSE:
 *   Prepare a Vibe Coding session without bypassing Framework context limits,
 *   provider policy, secret references or human patch approval.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/vibe_coding.h"

#include <stdio.h>
#include <string.h>

static UmiStatus copy_text(char *destination,
                           size_t capacity,
                           const char *source)
{
    int written;
    if (destination == NULL || capacity == 0U || source == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    written = snprintf(destination, capacity, "%s", source);
    return written >= 0 && (size_t)written < capacity
        ? UMI_STATUS_OK : UMI_STATUS_CAPACITY_EXCEEDED;
}

UmiStatus umi_studio_vibe_coding_prepare(
    UmiStudioAiPlatform *platform,
    const char *request_id,
    const char *goal,
    const char *active_path,
    UmiStudioVibeCodingSession *out_session)
{
    UmiStudioAiWorkbenchProfile profile;
    UmiStatus status;
    if (platform == NULL || request_id == NULL || request_id[0] == '\0' ||
        goal == NULL || goal[0] == '\0' || active_path == NULL ||
        active_path[0] == '\0' || out_session == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    (void)memset(out_session, 0, sizeof(*out_session));
    out_session->struct_size = (uint32_t)sizeof(*out_session);
    out_session->api_version = 1U;
    out_session->state = UMI_STUDIO_VIBE_CODING_IDLE;
    status = umi_studio_ai_platform_workbench_profile(platform, &profile);
    if (status == UMI_STATUS_OK) {
        status = copy_text(out_session->request_id,
                           sizeof(out_session->request_id), request_id);
    }
    if (status == UMI_STATUS_OK) {
        status = copy_text(out_session->goal,
                           sizeof(out_session->goal), goal);
    }
    if (status == UMI_STATUS_OK) {
        status = copy_text(out_session->active_path,
                           sizeof(out_session->active_path), active_path);
    }
    if (status == UMI_STATUS_OK) {
        status = copy_text(out_session->runtime_id,
                           sizeof(out_session->runtime_id),
                           profile.preferred_runtime_id);
    }
    if (status == UMI_STATUS_OK) {
        status = copy_text(
            out_session->model_id, sizeof(out_session->model_id),
            profile.remote_configured ? profile.remote_model_id
                                      : "studio-reference");
    }
    if (status == UMI_STATUS_OK) {
        status = copy_text(
            out_session->secret_reference,
            sizeof(out_session->secret_reference),
            profile.remote_configured ? profile.remote_secret_reference : "");
    }
    if (status != UMI_STATUS_OK) return status;

    status = umi_studio_coding_assistant_prepare(
        platform, request_id, UMI_AI_CODING_TASK_CHAT, goal, active_path,
        &out_session->plan);
    if (status != UMI_STATUS_OK) {
        out_session->state = UMI_STUDIO_VIBE_CODING_FAILED;
        out_session->revision = 1U;
        return status;
    }
    out_session->rag_enabled = profile.rag_enabled;
    out_session->stream_responses = profile.stream_responses;
    out_session->requires_patch_approval = profile.require_patch_approval;
    out_session->state = UMI_STUDIO_VIBE_CODING_CONTEXT_READY;
    out_session->revision = 1U;
    return UMI_STATUS_OK;
}

const char *umi_studio_vibe_coding_state_text(
    UmiStudioVibeCodingState state)
{
    switch (state) {
        case UMI_STUDIO_VIBE_CODING_IDLE: return "idle";
        case UMI_STUDIO_VIBE_CODING_CONTEXT_READY: return "context ready";
        case UMI_STUDIO_VIBE_CODING_AWAITING_MODEL: return "awaiting model";
        case UMI_STUDIO_VIBE_CODING_AWAITING_REVIEW: return "awaiting review";
        case UMI_STUDIO_VIBE_CODING_APPLYING: return "applying";
        case UMI_STUDIO_VIBE_CODING_VALIDATING: return "validating";
        case UMI_STUDIO_VIBE_CODING_COMPLETE: return "complete";
        case UMI_STUDIO_VIBE_CODING_FAILED: return "failed";
        default: return "unknown";
    }
}
