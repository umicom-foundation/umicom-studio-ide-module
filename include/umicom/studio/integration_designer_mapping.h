/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/integration_designer_mapping.h
 *
 * PURPOSE:
 *   Publish Studio's public integration designer mapping contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_INTEGRATION_DESIGNER_MAPPING_H
#define UMICOM_STUDIO_INTEGRATION_DESIGNER_MAPPING_H
#include "umicom/studio/integration_designer_centre.h"
/**
 * Provide the studio integration designer add mapping operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_integration_designer_add_mapping(UmiStudioIntegrationDesignerCentre *centre,const UmiIntegrationFieldMapping *mapping);
/**
 * Provide the studio integration designer find mapping operation used by this module and
 * its client applications.
 */
const UmiIntegrationFieldMapping *umi_studio_integration_designer_find_mapping(const UmiStudioIntegrationDesignerCentre *centre,const char *mapping_id);
/**
 * Provide the studio integration designer preview mapping operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_integration_designer_preview_mapping(const UmiStudioIntegrationDesignerCentre *centre,const char *mapping_id,const UmiIntegrationDesignerRecord *source,UmiIntegrationDesignerRecord *target,UmiIntegrationDesignerValidation *validation);
#endif
