/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/sdk.h
 *
 * PURPOSE:
 *   Expose Studio use cases for locating and validating installed Framework SDKs.
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
#ifndef UMICOM_STUDIO_SDK_H
#define UMICOM_STUDIO_SDK_H
#include "umicom/studio/compatibility_platform.h"
/**
 * Provide the studio sdk probe operation used by this module and its client applications.
 */
UmiStatus umi_studio_sdk_probe(const char *prefix,UmiSdkValidationResult *out_result);
#endif

