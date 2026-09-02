/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/helix_agents.h
 *
 * PURPOSE:
 *   List the specialist Helix agents registered for Studio.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Agent inspection lets the UI show roles and permissions instead of presenting Helix as one opaque autonomous process.
 */

#ifndef APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_HELIX_AGENTS_H
#define APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_HELIX_AGENTS_H

#include <stddef.h>
#include "umicom/studio/ai_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return the number of records represented by studio helix agent without changing their
 * state.
 */
size_t umi_studio_helix_agent_count(UmiStudioAiPlatform *platform);
/**
 * Find studio helix agent while leaving the underlying catalogue or model owned by this
 * module.
 */
const UmiHelixAgent *umi_studio_helix_agent_at(UmiStudioAiPlatform *platform,
                                               size_t index);

#ifdef __cplusplus
}
#endif

#endif
