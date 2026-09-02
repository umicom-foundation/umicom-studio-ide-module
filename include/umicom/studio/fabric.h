/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/fabric.h
 *
 * PURPOSE:
 *   Declare Studio Integration Fabric bootstrap schemas, topics and health verification shared by product frontends, controllers and native inspection tools.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_FABRIC_H
#define UMICOM_STUDIO_FABRIC_H

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio fabric register defaults operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_fabric_register_defaults(UmiStudioServices *services);
/**
 * Check that studio fabric satisfies its contract before another service relies on it.
 */
UmiStatus umi_studio_fabric_validate(UmiStudioServices *services);

#ifdef __cplusplus
}
#endif

#endif
