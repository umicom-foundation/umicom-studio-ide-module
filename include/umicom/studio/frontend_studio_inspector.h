/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/frontend_studio_inspector.h
 *
 * PURPOSE:
 *   Publish Studio's public frontend studio inspector contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_FRONTEND_STUDIO_INSPECTOR_H
#define UMICOM_STUDIO_FRONTEND_STUDIO_INSPECTOR_H
#include "umicom/studio/web_platform.h"
/**
 * Provide the studio frontend inspector capture node operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_frontend_inspector_capture_node(UmiStudioWebPlatform *platform,const UmiFrontendDomNode *node);
/**
 * Provide the studio frontend inspector select node operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_frontend_inspector_select_node(UmiStudioWebPlatform *platform,const char *node_id,UmiFrontendDomNode *out_node);
/**
 * Provide the studio frontend inspector record network operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_frontend_inspector_record_network(UmiStudioWebPlatform *platform,const UmiFrontendNetworkEntry *entry);
/**
 * Provide the studio frontend inspector record console operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_frontend_inspector_record_console(UmiStudioWebPlatform *platform,const UmiFrontendConsoleEntry *entry);
/**
 * Release or reset state held by studio frontend inspector so the same storage can be
 * reused safely.
 */
void umi_studio_frontend_inspector_clear(UmiStudioWebPlatform *platform);
#endif
