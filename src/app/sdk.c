/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/sdk.c
 *
 * PURPOSE:
 *   Validate an installed Framework SDK for Studio-generated and external projects.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio is the reference consumer. These adapters call public Framework ABI, SDK and product APIs rather than duplicating those mechanisms inside the IDE.
 */
#include "umicom/studio/sdk.h"
/* Provide the studio sdk probe operation used by this module and its client applications. */
UmiStatus umi_studio_sdk_probe(const char *prefix,UmiSdkValidationResult *o){UmiSdkRequirement r={{0U,5U,0U},UMICOM_FRAMEWORK_ABI_VERSION};return umi_sdk_validate_prefix(prefix,&r,o);}
