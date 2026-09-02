/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/delivery_update.h
 *
 * PURPOSE:
 *   Plan Studio updates and rollbacks using Framework generation and compatibility rules.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio can explain an update decision without owning the generic comparison or rollback algorithm.
 */

#ifndef APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_DELIVERY_UPDATE_H
#define APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_DELIVERY_UPDATE_H

#include "umicom/umicom.h"
#include "umicom/studio/delivery_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio update plan operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_update_plan(uint64_t current_generation,
                                    uint64_t available_generation,
                                    int compatible,
                                    int policy_allows,
                                    UmiUpdatePlan *out_plan);
/**
 * Provide the studio rollback plan operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_rollback_plan(uint64_t current_generation,
                                   uint64_t target_generation,
                                   const char *reason,
                                   UmiRollbackPlan *out_plan);

#ifdef __cplusplus
}
#endif

#endif
