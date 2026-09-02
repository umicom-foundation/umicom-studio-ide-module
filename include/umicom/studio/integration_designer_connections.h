/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/integration_designer_connections.h
 *
 * PURPOSE:
 *   Publish Studio's public integration designer connections contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_INTEGRATION_DESIGNER_CONNECTIONS_H
#define UMICOM_STUDIO_INTEGRATION_DESIGNER_CONNECTIONS_H
#include "umicom/studio/integration_designer_centre.h"
/**
 * Provide the studio integration designer add connection operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_integration_designer_add_connection(UmiStudioIntegrationDesignerCentre *centre,const UmiIntegrationConnectionProfile *profile);
/**
 * Provide the studio integration designer add schema operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_integration_designer_add_schema(UmiStudioIntegrationDesignerCentre *centre,const UmiIntegrationSchemaDescriptor *schema);
/**
 * Provide the studio integration designer find connection operation used by this module
 * and its client applications.
 */
const UmiIntegrationConnectionProfile *umi_studio_integration_designer_find_connection(const UmiStudioIntegrationDesignerCentre *centre,const char *profile_id);
#endif
