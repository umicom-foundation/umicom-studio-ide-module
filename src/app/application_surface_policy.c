/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/application_surface_policy.c
 *
 * PURPOSE:
 *   Adapt the shared Framework runtime policy to clear Studio operations and
 *   a compact diagnostic snapshot for user interfaces and support tools.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/application_surface_policy.h"

#include <string.h>

UmiStatus umi_studio_application_surface_policy_snapshot(
    const UmiStudioApplicationSurface *surface,
    UmiStudioApplicationSurfacePolicySnapshot *out_snapshot)
{
    UmiApplicationPresentationSurfaceSnapshot runtime_snapshot;
    const UmiApplicationPresentationWorkspaceRuntimePolicy *policy;
    UmiStatus status;
    if (surface == NULL || out_snapshot == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_studio_application_surface_snapshot(surface, &runtime_snapshot);
    if (status != UMI_STATUS_OK) return status;
    policy = runtime_snapshot.workspace_policy;
    if (policy == NULL) return UMI_STATUS_INVALID_STATE;
    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->recipe_id = runtime_snapshot.recipe_id;
    out_snapshot->startup_policy =
        umi_application_presentation_startup_policy_text(policy->startup_policy);
    out_snapshot->checkpoint_policy =
        umi_application_presentation_checkpoint_policy_text(
            policy->checkpoint_policy);
    out_snapshot->focus_policy =
        umi_application_presentation_focus_policy_text(policy->focus_policy);
    out_snapshot->background_policy =
        umi_application_presentation_background_policy_text(
            policy->background_policy);
    out_snapshot->checkpoint_interval_seconds =
        policy->checkpoint_interval_seconds;
    out_snapshot->scheduled_refresh_panels =
        runtime_snapshot.scheduled_refresh_count;
    out_snapshot->streaming_panels = runtime_snapshot.streaming_count;
    out_snapshot->guarded_command_panels =
        runtime_snapshot.guarded_command_count;
    out_snapshot->context_enabled_panels =
        runtime_snapshot.context_enabled_count;
    out_snapshot->restore_focus = policy->restore_focus;
    out_snapshot->share_context = policy->share_context;
    out_snapshot->allow_background_commands =
        policy->allow_background_commands;
    return UMI_STATUS_OK;
}

UmiStatus umi_studio_application_surface_policy_advance(
    UmiStudioApplicationSurface *surface,
    uint32_t elapsed_seconds)
{
    UmiApplicationPresentationSurfaceRuntime *runtime =
        umi_studio_application_surface_runtime(surface);
    return runtime != NULL
        ? umi_application_presentation_surface_runtime_advance(
              runtime, elapsed_seconds)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_policy_set_background(
    UmiStudioApplicationSurface *surface,
    int background)
{
    UmiApplicationPresentationSurfaceRuntime *runtime =
        umi_studio_application_surface_runtime(surface);
    return runtime != NULL
        ? umi_application_presentation_surface_runtime_set_background(
              runtime, background)
        : UMI_STATUS_INVALID_ARGUMENT;
}

UmiStatus umi_studio_application_surface_policy_context_changed(
    UmiStudioApplicationSurface *surface,
    const char *component_id,
    const char *context_value)
{
    UmiApplicationPresentationSurfaceRuntime *runtime =
        umi_studio_application_surface_runtime(surface);
    return runtime != NULL
        ? umi_application_presentation_surface_runtime_context_changed(
              runtime, component_id, context_value)
        : UMI_STATUS_INVALID_ARGUMENT;
}

int umi_studio_application_surface_policy_checkpoint_due(
    const UmiStudioApplicationSurface *surface,
    uint32_t elapsed_since_checkpoint_seconds,
    int changed)
{
    const UmiApplicationPresentationSurfaceRuntime *runtime =
        umi_studio_application_surface_runtime_const(surface);
    if (runtime == NULL) return 0;
    return umi_application_presentation_surface_runtime_checkpoint_due(
        runtime, elapsed_since_checkpoint_seconds, changed);
}
