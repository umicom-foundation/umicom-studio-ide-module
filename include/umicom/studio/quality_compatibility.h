/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/quality_compatibility.h
 *
 * PURPOSE:
 *   Publish Studio's public quality compatibility contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_QUALITY_COMPATIBILITY_H
#define UMICOM_STUDIO_QUALITY_COMPATIBILITY_H
#include "umicom/studio/quality_centre.h"
UmiStatus umi_studio_quality_compare_api(UmiStudioQualityCentre *centre,const UmiCodeGuardApiBaseline *expected,const UmiCodeGuardApiBaseline *current);
const UmiCodeGuardApiChange *umi_studio_quality_api_change_at(const UmiStudioQualityCentre *centre,size_t index);
#endif
