/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/application_surface_policy.h
 *
 * PURPOSE:
 *   Give Studio a small product-facing API for Framework-owned refresh,
 *   background, context and checkpoint behavior without duplicating it.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_APPLICATION_SURFACE_POLICY_H
#define UMICOM_STUDIO_APPLICATION_SURFACE_POLICY_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/studio/application_surface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio application surface policy snapshot data shared with callers of
 * this public contract.
 */
typedef struct UmiStudioApplicationSurfacePolicySnapshot {
    const char *recipe_id;
    const char *startup_policy;
    const char *checkpoint_policy;
    const char *focus_policy;
    const char *background_policy;
    uint32_t checkpoint_interval_seconds;
    size_t scheduled_refresh_panels;
    size_t streaming_panels;
    size_t guarded_command_panels;
    size_t context_enabled_panels;
    int restore_focus;
    int share_context;
    int allow_background_commands;
} UmiStudioApplicationSurfacePolicySnapshot;

/**
 * Provide the studio application surface policy snapshot operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_application_surface_policy_snapshot(
    const UmiStudioApplicationSurface *surface,
    UmiStudioApplicationSurfacePolicySnapshot *out_snapshot);
/**
 * Provide the studio application surface policy advance operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_application_surface_policy_advance(
    UmiStudioApplicationSurface *surface,
    uint32_t elapsed_seconds);
/**
 * Provide the studio application surface policy set background operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_application_surface_policy_set_background(
    UmiStudioApplicationSurface *surface,
    int background);
/**
 * Provide the studio application surface policy context changed operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_application_surface_policy_context_changed(
    UmiStudioApplicationSurface *surface,
    const char *component_id,
    const char *context_value);
/**
 * Provide the studio application surface policy checkpoint due operation used by this
 * module and its client applications.
 */
int umi_studio_application_surface_policy_checkpoint_due(
    const UmiStudioApplicationSurface *surface,
    uint32_t elapsed_since_checkpoint_seconds,
    int changed);

#ifdef __cplusplus
}
#endif

#endif
