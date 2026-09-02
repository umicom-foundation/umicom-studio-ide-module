/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/integration_designer_mapping.c
 *
 * PURPOSE:
 *   Implement the integration designer mapping behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Integration Designer mapping | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/integration_designer_mapping.h"
#include <string.h>

/*
 * Provide the studio integration designer add mapping operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_integration_designer_add_mapping(UmiStudioIntegrationDesignerCentre *centre,const UmiIntegrationFieldMapping *mapping)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_integration_designer_fabric_add_mapping(&centre->fabric,mapping);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) centre->revision += 1U;
    return status;
}

/*
 * Provide the studio integration designer find mapping operation used by this module and
 * its client applications.
 */
const UmiIntegrationFieldMapping *umi_studio_integration_designer_find_mapping(const UmiStudioIntegrationDesignerCentre *centre,const char *mapping_id)
{
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || mapping_id == NULL) return NULL;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < centre->fabric.mapping_count; ++index) /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (strcmp(centre->fabric.mappings[index].id,mapping_id) == 0) return &centre->fabric.mappings[index];
    return NULL;
}

/*
 * Provide the studio integration designer preview mapping operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_integration_designer_preview_mapping(const UmiStudioIntegrationDesignerCentre *centre,const char *mapping_id,const UmiIntegrationDesignerRecord *source,UmiIntegrationDesignerRecord *target,UmiIntegrationDesignerValidation *validation)
{
    const UmiIntegrationFieldMapping *mapping = umi_studio_integration_designer_find_mapping(centre,mapping_id);
    return mapping == NULL ? UMI_STATUS_NOT_FOUND : umi_integration_field_mapping_apply(mapping,source,target,validation);
}
