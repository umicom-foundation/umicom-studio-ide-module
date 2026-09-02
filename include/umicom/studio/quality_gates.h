/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/quality_gates.h
 *
 * PURPOSE:
 *   Publish Studio's public quality gates contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_QUALITY_GATES_H
#define UMICOM_STUDIO_QUALITY_GATES_H
#include "umicom/studio/quality_centre.h"
/**
 * Provide the studio quality set policy operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_quality_set_policy(UmiStudioQualityCentre *centre,const UmiCodeGuardQualityPolicy *policy);
/**
 * Provide the studio quality evaluate operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_quality_evaluate(UmiStudioQualityCentre *centre,UmiCodeGuardQualityEvaluation *out_evaluation);
#endif
