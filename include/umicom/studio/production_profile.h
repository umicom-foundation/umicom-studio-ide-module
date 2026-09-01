/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/production_profile.h
 *
 * PURPOSE:
 *   Project the Framework-owned application production control plane into this
 *   thin product without duplicating layout, readiness or acceptance logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_PRODUCTION_PROFILE_H
#define UMICOM_STUDIO_PRODUCTION_PROFILE_H
#include "umicom/application/production/production.h"
#include "umicom/studio/productisation_contribution.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UmiStudioProductionProfile {
    UmiApplicationProductionBinding binding;
    const char *primary_workspace_id;
    const char *default_command_category;
} UmiStudioProductionProfile;
/** Resolve Studio's canonical experience into a thin production binding. */
UmiStatus umi_studio_production_profile_build(
    UmiStudioProductionProfile *out_profile);
#ifdef __cplusplus
}
#endif
#endif

