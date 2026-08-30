/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/quality_remediation.h
 *
 * PURPOSE:
 *   Publish Studio's public quality remediation contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_QUALITY_REMEDIATION_H
#define UMICOM_STUDIO_QUALITY_REMEDIATION_H
#include "umicom/studio/quality_centre.h"
UmiStatus umi_studio_quality_remediation_refresh(UmiStudioQualityCentre *centre);
const UmiCodeGuardRemediationAction *umi_studio_quality_remediation_at(const UmiStudioQualityCentre *centre,size_t index);
#endif
