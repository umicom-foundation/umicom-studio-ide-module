/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/sdk_centre.h
 *
 * PURPOSE:
 *   Present installed Framework SDK exports to Studio project consumers.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SDK_CENTRE_H
#define UMICOM_STUDIO_SDK_CENTRE_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio sdk centre data shared with callers of this public contract.
 */
typedef struct UmiStudioSdkCentre {
    UmiSdkExportPlan export_plan;
    char consumer_cmake[2048];
    int ready;
} UmiStudioSdkCentre;

/**
 * Initialise studio sdk centre from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_sdk_centre_init(UmiStudioSdkCentre *centre,
                                         const char *install_prefix);
/**
 * Provide the studio sdk centre refresh operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_sdk_centre_refresh(UmiStudioSdkCentre *centre);
/**
 * Provide the studio sdk centre consumer cmake operation used by this module and its
 * client applications.
 */
const char *umi_studio_sdk_centre_consumer_cmake(
    const UmiStudioSdkCentre *centre);

#ifdef __cplusplus
}
#endif
#endif
