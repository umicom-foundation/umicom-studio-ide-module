/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/developer_session.h
 * PURPOSE: Track the active developer context shared by Studio workbench areas.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DEVELOPER_SESSION_H
#define UMICOM_STUDIO_DEVELOPER_SESSION_H
#include <stdint.h>
#include "umicom/base/status.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio developer session data shared with callers of this public contract.
 */
typedef struct UmiStudioDeveloperSession UmiStudioDeveloperSession;
/**
 * Represent the studio developer session snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDeveloperSessionSnapshot {uint32_t struct_size;uint32_t api_version;char project_id[128];char configuration_id[128];char launch_profile_id[128];char repository_id[128];char test_item_id[128];char active_file[512];uint64_t revision;} UmiStudioDeveloperSessionSnapshot;
/**
 * Initialise studio developer session from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_developer_session_create(UmiStudioDeveloperSession **out_session);
/**
 * Release or reset state held by studio developer session so the same storage can be
 * reused safely.
 */
void umi_studio_developer_session_destroy(UmiStudioDeveloperSession *session);
/**
 * Provide the studio developer session set context operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_session_set_context(UmiStudioDeveloperSession *session,const UmiStudioDeveloperSessionSnapshot *context);
/**
 * Provide the studio developer session snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_session_snapshot(const UmiStudioDeveloperSession *session,UmiStudioDeveloperSessionSnapshot *out_snapshot);
#ifdef __cplusplus
}
#endif
#endif
