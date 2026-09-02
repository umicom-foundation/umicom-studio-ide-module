/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/helix_agent_operations.h
 * PURPOSE: Build canonical filesystem, build, test and review action descriptors.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Canonical constructors keep action identifiers and retry
 * limits consistent across command palette, tools and workbench views. */
#ifndef UMICOM_STUDIO_HELIX_AGENT_OPERATIONS_H
#define UMICOM_STUDIO_HELIX_AGENT_OPERATIONS_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio helix agent filesystem action operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_helix_agent_filesystem_action(
    const char *target, UmiHelixAction *out_action);
/**
 * Provide the studio helix agent build action operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_helix_agent_build_action(
    const char *target, UmiHelixAction *out_action);
/**
 * Provide the studio helix agent test action operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_helix_agent_test_action(
    const char *target, UmiHelixAction *out_action);
/**
 * Provide the studio helix agent review action operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_helix_agent_review_action(
    const char *target, UmiHelixAction *out_action);
/**
 * Provide the studio helix agent source control action operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_helix_agent_source_control_action(
    const char *target, UmiHelixAction *out_action);

#ifdef __cplusplus
}
#endif

#endif
