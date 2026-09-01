/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/workspace_profiles.h
 *
 * PURPOSE:
 *   Declare Studio's named profiles for the live Framework workbench and the
 *   canonical Application Suite profiles projected into that same model.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_WORKSPACE_PROFILES_H
#define UMICOM_STUDIO_WORKSPACE_PROFILES_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_WORKSPACE_PROFILE_DEVELOP "develop"
#define UMI_STUDIO_WORKSPACE_PROFILE_FOCUS "focus"
#define UMI_STUDIO_WORKSPACE_PROFILE_DEBUG "debug"
#define UMI_STUDIO_WORKSPACE_PROFILE_SOURCE_CONTROL "source-control"
#define UMI_STUDIO_WORKSPACE_PROFILE_TESTING "testing"
#define UMI_STUDIO_WORKSPACE_PROFILE_BUILD "build"
#define UMI_STUDIO_WORKSPACE_PROFILE_REVIEW "review"
#define UMI_STUDIO_WORKSPACE_PROFILE_OPERATIONS "operations"
#define UMI_STUDIO_WORKSPACE_PROFILE_TRADING "trading"

#define UMI_STUDIO_BUILTIN_WORKSPACE_PROFILE_COUNT 9U
#define UMI_STUDIO_CANONICAL_WORKSPACE_PROFILE_COUNT 6U
#define UMI_STUDIO_WORKSPACE_PROFILE_COUNT \
    (UMI_STUDIO_BUILTIN_WORKSPACE_PROFILE_COUNT + \
     UMI_STUDIO_CANONICAL_WORKSPACE_PROFILE_COUNT)

/** Return the live total so frontends follow catalogue growth automatically. */
size_t umi_studio_workspace_profile_count(void);
/** Register Studio chrome profiles and canonical layouts in one workbench. */
UmiStatus umi_studio_workspace_profiles_register(UmiUiWorkbench *workbench);

#ifdef __cplusplus
}
#endif

#endif
