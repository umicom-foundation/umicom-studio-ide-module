/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/runtime-manager/include/umicom/studio/runtime_launch_plan.h
 *
 * PURPOSE:
 *   Create a non-mutating launch plan before process lifecycle execution is implemented.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of the Studio Application Hub / Runtime Manager.
 * The code is intentionally explicit and bounded so a new C developer can
 * follow the data flow without hidden ownership or private product coupling.
 */

#ifndef UMICOM_STUDIO_RUNTIME_LAUNCH_PLAN_H
#define UMICOM_STUDIO_RUNTIME_LAUNCH_PLAN_H

#include <stdbool.h>

#include "umicom/base/status.h"
#include "umicom/studio/runtime_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * List the named studio runtime launch readiness values accepted by this public contract.
 */
typedef enum UmiStudioRuntimeLaunchReadiness {
    UMI_STUDIO_RUNTIME_LAUNCH_READY = 0,
    UMI_STUDIO_RUNTIME_LAUNCH_ALREADY_RUNNING,
    UMI_STUDIO_RUNTIME_LAUNCH_EXECUTABLE_UNKNOWN,
    UMI_STUDIO_RUNTIME_LAUNCH_APPLICATION_UNKNOWN
} UmiStudioRuntimeLaunchReadiness;

/**
 * Represent the studio runtime launch plan data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioRuntimeLaunchPlan {
    char application_id[UMI_INTEGRATION_ID_CAPACITY];
    char application_name[UMI_INTEGRATION_NAME_CAPACITY];
    char executable[UMI_INTEGRATION_PATH_CAPACITY];
    UmiStudioRuntimeLaunchReadiness readiness;
    bool can_launch;
    char reason[UMI_INTEGRATION_TEXT_CAPACITY];
} UmiStudioRuntimeLaunchPlan;

/**
 * Provide the studio runtime plan launch operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_runtime_plan_launch(
    const UmiStudioRuntimeManager *manager,
    const char *application_id,
    UmiStudioRuntimeLaunchPlan *plan
);

/**
 * Provide the studio runtime launch readiness text operation used by this module and its
 * client applications.
 */
const char *umi_studio_runtime_launch_readiness_text(
    UmiStudioRuntimeLaunchReadiness readiness
);

#ifdef __cplusplus
}
#endif

#endif
