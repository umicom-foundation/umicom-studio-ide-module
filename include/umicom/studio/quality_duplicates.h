/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/quality_duplicates.h
 *
 * PURPOSE:
 *   Publish Studio's public quality duplicates contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_QUALITY_DUPLICATES_H
#define UMICOM_STUDIO_QUALITY_DUPLICATES_H
#include "umicom/studio/quality_centre.h"
/**
 * Add studio quality duplicate only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_quality_duplicate_add(UmiStudioQualityCentre *centre,const UmiCodeGuardDuplicateFile *first,const UmiCodeGuardDuplicateFile *second);
/**
 * Provide the studio quality duplicate decide operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_quality_duplicate_decide(UmiStudioQualityCentre *centre,size_t index,UmiCodeGuardCandidateDisposition disposition);
/**
 * Find studio quality duplicate while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiCodeGuardDuplicateCandidate *umi_studio_quality_duplicate_at(const UmiStudioQualityCentre *centre,size_t index);
#endif
